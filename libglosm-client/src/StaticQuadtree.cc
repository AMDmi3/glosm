/*
 * Copyright (C) 2010-2011 Dmitry Marakasov
 *
 * This file is part of glosm.
 *
 * glosm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glosm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glosm/StaticQuadtree.hh>

#if defined(__APPLE__)
#	include <OpenGL/gl.h>
#else
#	include <GL/gl.h>
#endif

#include <glosm/Viewer.hh>
#include <glosm/Geometry.hh>
#include <glosm/GeometryDatasource.hh>
#include <glosm/Tile.hh>

#include <stdexcept>

StaticQuadtree::StaticQuadtree(const Projection projection, const GeometryDatasource& ds): projection_(projection), datasource_(ds), root_(new Node()) {
	if (pthread_mutex_init(&loading_queue_mutex_, 0) != 0)
		throw std::runtime_error("pthread_mutex_init failed");

	if (pthread_cond_init(&loading_queue_cond_, 0) != 0) {
		pthread_mutex_destroy(&loading_queue_mutex_);
		throw std::runtime_error("pthread_cond_init failed");
	}

	if (pthread_create(&loading_thread_, NULL, LoadingThreadFuncWrapper, (void*)this) != 0) {
		pthread_cond_destroy(&loading_queue_cond_);
		pthread_mutex_destroy(&loading_queue_mutex_);
		throw std::runtime_error("pthread_create failed");
	}

	target_level_ = 16;
	generation_ = 0;
	loading_thread_die_ = false;
}

StaticQuadtree::~StaticQuadtree() {
	loading_thread_die_ = true;
	pthread_cond_signal(&loading_queue_cond_);

	pthread_join(loading_thread_, NULL);
	pthread_cond_destroy(&loading_queue_cond_);
	pthread_mutex_destroy(&loading_queue_mutex_);

	DestroyNodes(root_);
}

/*
 * recursive quadtree processing
 */

void StaticQuadtree::DestroyNodes(Node* node) {
	delete node->tile;

	for (int d = 0; d < 4; ++d)
		if (node->child[d])
			DestroyNodes(node->child[d]);

	delete node;
}

void StaticQuadtree::RenderNodes(Node* node, const Viewer& viewer) const {
	if (node->generation != generation_)
		return;

	if (node->tile) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		Vector3f offset = projection_.Project(node->tile->GetReference(), viewer.GetPos(projection_));
		glTranslatef(offset.x, offset.y, offset.z);

		node->tile->Render();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		return;
	}

	for (int d = 0; d < 4; ++d)
		if (node->child[d])
			RenderNodes(node->child[d], viewer);
}

void StaticQuadtree::LoadNodes(Node* node, const BBoxi& bbox, bool sync, int level, int x, int y) {
	node->generation = generation_;

	if (level == target_level_) {
		/* leaf */
		if (node->tile == NULL) {
			if (sync) {
				BBoxi bbox = BBoxi::ForGeoTile(level, x, y);
				Geometry geom;
				datasource_.GetGeometry(geom, bbox);
				node->tile = SpawnTile(geom, bbox);
			} else {
				EnqueueTile(node, level, x, y);
			}
		}
		return;
	}

	/* children */
	for (int d = 0; d < 4; ++d) {
		int xx = x * 2 + d % 2;
		int yy = y * 2 + d / 2;
		if (BBoxi::ForGeoTile(level + 1, xx, yy).Intersects(bbox)) {
			if (!node->child[d])
				node->child[d] = new Node;
			LoadNodes(node->child[d], bbox, sync, level + 1, xx, yy);
		}
	}
}

void StaticQuadtree::SweepNodes(Node* node) {
	for (int d = 0; d < 4; ++d) {
		if (node->child[d]) {
			if (node->child[d]->generation != generation_) {
				DestroyNodes(node->child[d]);
				node->child[d] = NULL;
			} else {
				SweepNodes(node->child[d]);
			}
		}
	}
}

/*
 * loading queue - related
 */

void StaticQuadtree::EnqueueTile(Node* node, int level, int x, int y) {
	pthread_mutex_lock(&loading_queue_mutex_);

	size_t size = loading_queue_.size();

	loading_queue_.push_front(LoadingTask(level, x, y, generation_));

	/* wakeup thread */
	if (size == 0)
		pthread_cond_signal(&loading_queue_cond_);

	pthread_mutex_unlock(&loading_queue_mutex_);
}

void StaticQuadtree::CleanupQueue() {
	pthread_mutex_lock(&loading_queue_mutex_);

	while (loading_queue_.size() > 0 && loading_queue_.back().generation != generation_)
		loading_queue_.pop_back();

	pthread_mutex_unlock(&loading_queue_mutex_);
}

void StaticQuadtree::LoadingThreadFunc() {
	while (!loading_thread_die_) {
		pthread_mutex_lock(&loading_queue_mutex_);

		if (loading_queue_.size() == 0)
			pthread_cond_wait(&loading_queue_cond_, &loading_queue_mutex_);

		if (loading_queue_.size() == 0) {
			pthread_mutex_unlock(&loading_queue_mutex_);
			continue;
		}

		LoadingTask task = loading_queue_.front();
		loading_queue_.pop_front();

		pthread_mutex_unlock(&loading_queue_mutex_);

		BBoxi bbox = BBoxi::ForGeoTile(task.level, task.x, task.y);
		Geometry geom;
		datasource_.GetGeometry(geom, bbox);

		//task.node->tile = SpawnTile(geom, bbox);
	}
}

void* StaticQuadtree::LoadingThreadFuncWrapper(void* arg) {
	static_cast<StaticQuadtree*>(arg)->LoadingThreadFunc();
	return NULL;
}

/*
 * protected interface
 */

void StaticQuadtree::Render(const Viewer& viewer) const {
	RenderNodes(root_, viewer);
}

/*
 * public interface
 */

void StaticQuadtree::SetTargetLevel(int level) {
	target_level_ = level;
}

void StaticQuadtree::RequestVisible(const BBoxi& bbox, bool sync) {
	++generation_;
	LoadNodes(root_, bbox, sync);
	CleanupQueue();
}

void StaticQuadtree::GarbageCollect() {
	SweepNodes(root_);
}
