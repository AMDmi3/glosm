/*
 * Copyright (C) 2010-2011 Dmitry Marakasov
 *
 * This file is part of glosm.
 *
 * glosm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * glosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with glosm.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <glosm/TileManager.hh>

#include <glosm/Viewer.hh>
#include <glosm/Geometry.hh>
#include <glosm/GeometryDatasource.hh>
#include <glosm/GeometryOperations.hh>
#include <glosm/Tile.hh>
#include <glosm/Exception.hh>

#include <glosm/util/gl.h>

#include <algorithm>
#include <cstdio>

TileManager::TileManager(const Projection projection): projection_(projection), loading_(-1, -1, -1) {
	generation_ = 0;
	thread_die_flag_ = false;

	int errn;

	if ((errn = pthread_mutex_init(&tiles_mutex_, 0)) != 0)
		throw SystemError(errn) << "pthread_mutex_init failed";

	if ((errn = pthread_mutex_init(&queue_mutex_, 0)) != 0) {
		pthread_mutex_destroy(&tiles_mutex_);
		throw SystemError(errn) << "pthread_mutex_init failed";
	}

	if ((errn = pthread_cond_init(&queue_cond_, 0)) != 0) {
		pthread_mutex_destroy(&tiles_mutex_);
		pthread_mutex_destroy(&queue_mutex_);
		throw SystemError(errn) << "pthread_cond_init failed";
	}

	if ((errn = pthread_create(&loading_thread_, NULL, LoadingThreadFuncWrapper, (void*)this)) != 0) {
		pthread_mutex_destroy(&tiles_mutex_);
		pthread_mutex_destroy(&queue_mutex_);
		pthread_cond_destroy(&queue_cond_);
		throw SystemError(errn) << "pthread_create failed";
	}

	level_ = 12;
	range_ = 1000.0f;
	flags_ = 0;
	height_effect_ = false;

	total_size_ = 0;
	tile_count_ = 0;
}


TileManager::~TileManager() {
	thread_die_flag_ = true;
	pthread_cond_signal(&queue_cond_);

	/* @todo check exit code? */
	pthread_join(loading_thread_, NULL);

	pthread_cond_destroy(&queue_cond_);
	pthread_mutex_destroy(&queue_mutex_);
	pthread_mutex_destroy(&tiles_mutex_);

	fprintf(stderr, "Tile statistics before cleanup: %u tiles, %u bytes\n", (unsigned int)tile_count_, (unsigned int)total_size_);
	RecDestroyTiles(&root_);
}

/*
 * recursive quadtree processing
 */

void TileManager::RecLoadTilesBBox(RecLoadTilesInfo& info, QuadNode** pnode, int level, int x, int y) {
	QuadNode* node;

	if (*pnode == NULL) {
		/* no node; check if it's in bbox and if yes, create it */
		BBoxi bbox = BBoxi::ForGeoTile(level, x, y);
		if (!info.bbox->Intersects(bbox))
			return;
		node = *pnode = new QuadNode;
		node->bbox = bbox;
	} else {
		/* node exists, visit it if it's in bbox */
		node = *pnode;
		if (!info.bbox->Intersects(node->bbox))
			return;
	}
	/* range check passed and node exists */

	node->generation = generation_;

	if (level == level_) {
		if (node->tile)
			return; /* tile already loaded */

		if (info.flags & SYNC) {
			node->tile = SpawnTile(node->bbox, flags_);
			tile_count_++;
			total_size_ += node->tile->GetSize();
		} else if (loading_ != TileId(level, x, y)) {
			if (info.queue_size < 100) {
				queue_.push_front(TileTask(TileId(level, x, y), node->bbox));
				info.queue_size++;
			}
		}

		/* no more recursion is needed */
		return;
	}

	/* recurse */
	RecLoadTilesBBox(info, node->childs, level+1, x * 2, y * 2);
	RecLoadTilesBBox(info, node->childs + 1, level+1, x * 2 + 1, y * 2);
	RecLoadTilesBBox(info, node->childs + 2, level+1, x * 2, y * 2 + 1);
	RecLoadTilesBBox(info, node->childs + 3, level+1, x * 2 + 1, y * 2 + 1);

	return;
}

void TileManager::RecLoadTilesLocality(RecLoadTilesInfo& info, QuadNode** pnode, int level, int x, int y) {
	QuadNode* node;
	float thisdist;

	if (*pnode == NULL) {
		/* no node; check if it's in view and if yes, create it */
		BBoxi bbox = BBoxi::ForGeoTile(level, x, y);
		thisdist = ApproxDistanceSquare(bbox, info.viewer_pos);
		if (thisdist > range_ * range_)
			return;
		node = *pnode = new QuadNode;
		node->bbox = bbox;
	} else {
		/* node exists, visit it if it's in view */
		node = *pnode;
		thisdist = ApproxDistanceSquare(node->bbox, info.viewer_pos);
		if (thisdist > range_ * range_)
			return;
	}
	/* range check passed and node exists */

	node->generation = generation_;

	if (level == level_) {
		if (node->tile)
			return; /* tile already loaded */

		if (info.flags & SYNC) {
			node->tile = SpawnTile(node->bbox, flags_);
			tile_count_++;
			total_size_ += node->tile->GetSize();
		} else if (loading_ != TileId(level, x, y)) {
			if (queue_.empty()) {
				info.closest_distance = thisdist;
				queue_.push_front(TileTask(TileId(level, x, y), node->bbox));
				info.queue_size++;
			} else {
				if (thisdist < info.closest_distance) {
					/* this tile is closer than the best we have in the queue - push to front so it's downloaded faster */
					queue_.push_front(TileTask(TileId(level, x, y), node->bbox));
					info.closest_distance = thisdist;
					info.queue_size++;
				} else if (info.queue_size < 100) {
					/* push into the back of the queue, but not if queue is too long */
					queue_.push_back(TileTask(TileId(level, x, y), node->bbox));
					info.queue_size++;
				}
			}
		}

		/* no more recursion is needed */
		return;
	}

	/* recurse */
	RecLoadTilesLocality(info, node->childs, level+1, x * 2, y * 2);
	RecLoadTilesLocality(info, node->childs + 1, level+1, x * 2 + 1, y * 2);
	RecLoadTilesLocality(info, node->childs + 2, level+1, x * 2, y * 2 + 1);
	RecLoadTilesLocality(info, node->childs + 3, level+1, x * 2 + 1, y * 2 + 1);

	return;
}

void TileManager::RecPlaceTile(QuadNode* node, Tile* tile, int level, int x, int y) {
	if (node == NULL) {
		/* part of quadtree was garbage collected -> tile
		 * is no longer needed and should just be dropped */
		delete tile;
		return;
	}

	if (level == 0) {
		if (node->tile != NULL) {
			/* tile already loaded for some reason (sync loading?)
			 * -> drop copy */
			delete tile;
			return;
		}
		node->tile = tile;
		tile_count_++;
		total_size_ += tile->GetSize();
	} else {
		int mask = 1 << (level-1);
		int nchild = (!!(y & mask) << 1) | !!(x & mask);
		RecPlaceTile(node->childs[nchild], tile, level-1, x, y);
	}
}

void TileManager::RecDestroyTiles(QuadNode* node) {
	if (!node)
		return;

	if (node->tile) {
		tile_count_--;
		total_size_ -= node->tile->GetSize();
		delete node->tile;
		node->tile = NULL;
	}

	for (int i = 0; i < 4; ++i) {
		RecDestroyTiles(node->childs[i]);
		if (node->childs[i]) {
			delete node->childs[i];
			node->childs[i] = NULL;
		}
	}
}

void TileManager::RecGarbageCollectTiles(QuadNode* node, GCQueue& gcqueue) {
	/* simplest garbage collection that drops all inactive
	 * tiles. This should become much more clever */
	for (int i = 0; i < 4; ++i) {
		if (node->childs[i] == NULL)
			continue;

		if (node->childs[i]->generation != generation_) {
			gcqueue.push_back(&node->childs[i]);
		} else {
			RecGarbageCollectTiles(node->childs[i], gcqueue);
		}
	}
}

int TileManager::RecRenderTiles(QuadNode* node, const Viewer& viewer) {
	if (!node || node->generation != generation_)
		return 0;

	/* traverse tree depth-first */
	int childs = 0;
	childs += RecRenderTiles(node->childs[0], viewer);
	childs += RecRenderTiles(node->childs[1], viewer);
	childs += RecRenderTiles(node->childs[2], viewer);
	childs += RecRenderTiles(node->childs[3], viewer);

	/* ...and do not render a tile where all its childs were
	 * fully rendered */
	if (childs == 4)
		return 1;

	/* no tile */
	if (!node->tile)
		return 0;

	/* empty tile */
	if (node->tile->GetSize() == 0)
		return 1;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	/* prepare modelview matrix for the tile: position
	 * it in the right place given that viewer is always
	 * at (0, 0, 0) */
	Vector3f offset = projection_.Project(node->tile->GetReference(), Vector2i(viewer.GetPos(projection_))) +
			projection_.Project(Vector2i(viewer.GetPos(projection_)), viewer.GetPos(projection_));

	glTranslatef(offset.x, offset.y, offset.z);

	/* same for rotation */
	Vector3i ref = node->tile->GetReference();
	Vector3i pos = viewer.GetPos(projection_);

	/* normal at tile's reference point */
	Vector3d refnormal = (
			(Vector3d)projection_.Project(Vector3i(ref.x, ref.y, std::numeric_limits<osmint_t>::max()), pos) -
			(Vector3d)projection_.Project(Vector3i(ref.x, ref.y, 0), pos)
		).Normalized();

	/* normal at north pole */
	Vector3d polenormal = (
			(Vector3d)projection_.Project(Vector3i(ref.x, 900000000, std::numeric_limits<osmint_t>::max()), pos) -
			(Vector3d)projection_.Project(Vector3i(ref.x, 900000000, 0), pos)
		).Normalized();

	/* @todo IsValid() check basically detects
	 * MercatorProjection and does no rotation for it.
	 * While is's ok for now, this may need more generic
	 * approach in future */
	if (polenormal.IsValid()) {
		Vector3d side = refnormal.CrossProduct(polenormal).Normalized();

		glRotatef((double)((osmlong_t)ref.y - (osmlong_t)pos.y) / 10000000.0, side.x, side.y, side.z);
		glRotatef((double)((osmlong_t)ref.x - (osmlong_t)pos.x) / 10000000.0, polenormal.x, polenormal.y, polenormal.z);
	}

	/* @todo make it return bool and check return value,
	 * tile may be half-ready here */
	node->tile->Render();

#if defined(DEBUG_TILING) && !defined(WITH_GLES) && !defined(WITH_GLES2)
	Vector3f bound_1[4];
	Vector3f bound_2[40];

    bound_1[0] = projection_.Project(node->bbox.GetTopLeft(), ref);
    bound_1[1] = projection_.Project(node->bbox.GetTopRight(), ref);
    bound_1[2] = projection_.Project(node->bbox.GetBottomRight(), ref);
    bound_1[3] = projection_.Project(node->bbox.GetBottomLeft(), ref);

    bound_2[0] = projection_.Project(node->bbox.GetTopLeft(), ref);
    for (int i = 1; i < 10; i++)
        bound_2[i] = projection_.Project((Vector3d)node->bbox.GetTopLeft() * (double)(10 - i)*0.1 + (Vector3d)node->bbox.GetTopRight() * (double)(i)*0.1, ref);

    bound_2[10] = projection_.Project(node->bbox.GetTopRight(), ref);
    for (int i = 1; i < 10; i++)
        bound_2[i+10] = projection_.Project((Vector3d)node->bbox.GetTopRight() * (double)(10 - i)*0.1 + (Vector3d)node->bbox.GetBottomRight() * (double)(i)*0.1, ref);

    bound_2[20] = projection_.Project(node->bbox.GetBottomRight(), ref);
    for (int i = 1; i < 10; i++)
        bound_2[i+20] = projection_.Project((Vector3d)node->bbox.GetBottomRight() * (double)(10 - i)*0.1 + (Vector3d)node->bbox.GetBottomLeft() * (double)(i)*0.1, ref);

    bound_2[30] = projection_.Project(node->bbox.GetBottomLeft(), ref);
    for (int i = 1; i < 10; i++)
        bound_2[i+30] = projection_.Project((Vector3d)node->bbox.GetBottomLeft() * (double)(10 - i)*0.1 + (Vector3d)node->bbox.GetTopLeft() * (double)(i)*0.1, ref);

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 4; i++)
        glVertex3f(bound_1[i].x, bound_1[i].y, bound_1[i].z);
    glEnd();

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 40; i++)
        glVertex3f(bound_2[i].x, bound_2[i].y, bound_2[i].z);
    glEnd();

    glColor3f(0.5, 0.0, 0.0);
    glBegin(GL_LINES);
    for (int i = 0; i < 4; i++) {
        Vector3f nearref = bound_1[i] * 0.1;
        glVertex3f(nearref.x, nearref.y, nearref.z);
        glVertex3f(0.0f, 0.0f, 0.0f);
    }
    glEnd();
#endif

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	return 1;
}

/*
 * loading queue - related
 */

void TileManager::LoadingThreadFunc() {
	pthread_mutex_lock(&queue_mutex_);
	while (!thread_die_flag_) {
		/* found nothing, sleep */
		if (queue_.empty()) {
			pthread_cond_wait(&queue_cond_, &queue_mutex_);
			continue;
		}

		/* take a task from the queue */
		TileTask task = queue_.front();
		queue_.pop_front();

		/* mark it as loading */
		loading_ = task.id;

		pthread_mutex_unlock(&queue_mutex_);

		/* load tile */
		Tile* tile = SpawnTile(task.bbox, flags_);

		pthread_mutex_lock(&tiles_mutex_);
		RecPlaceTile(&root_, tile, task.id.level, task.id.x, task.id.y);

		pthread_mutex_unlock(&tiles_mutex_);

		/* The following happens:
		 * - main thread finishes Render and unlocks tiles_mutex_
		 * - this thread wakes up and loads MANY tiles without main
		 *   thread able to run
		 * - main thread finally runs after ~0.1 sec delay, which
		 *   is noticeable lag in realtime renderer
		 *
		 * Nut sure yet how to fix ot properly, but this sched_yield
		 * works for now.
		 */
		sched_yield();

		pthread_mutex_lock(&queue_mutex_);
		loading_ = TileId(-1, -1, -1);
	}
	pthread_mutex_unlock(&queue_mutex_);
}

void* TileManager::LoadingThreadFuncWrapper(void* arg) {
	static_cast<TileManager*>(arg)->LoadingThreadFunc();
	return NULL;
}

/*
 * protected interface
 */

void TileManager::Render(const Viewer& viewer) {
	pthread_mutex_lock(&tiles_mutex_);
	RecRenderTiles(&root_, viewer);
	pthread_mutex_unlock(&tiles_mutex_);
}

void TileManager::Load(RecLoadTilesInfo& info) {
	QuadNode* root = &root_;

	/* @todo add guard here instead of implicit locking,
	 * so we don't deadlock on exception */
	if (!(info.flags & SYNC)) {
		pthread_mutex_lock(&queue_mutex_);
		queue_.clear();
	}

	pthread_mutex_lock(&tiles_mutex_);

	switch (info.mode) {
	case RecLoadTilesInfo::BBOX:
		RecLoadTilesBBox(info, &root);
		break;
	case RecLoadTilesInfo::LOCALITY:
		info.viewer_pos = height_effect_ ? info.viewer->GetPos(projection_) : info.viewer->GetPos(projection_).Flattened();
		RecLoadTilesLocality(info, &root);
		break;
	}

	pthread_mutex_unlock(&tiles_mutex_);

	if (!(info.flags & SYNC)) {
		pthread_mutex_unlock(&queue_mutex_);

		if (!queue_.empty())
			pthread_cond_signal(&queue_cond_);
	}
}

/*
 * public interface
 */

void TileManager::LoadArea(const BBoxi& bbox, int flags) {
	RecLoadTilesInfo info;

	info.bbox = &bbox;
	info.flags = flags;
	info.mode = RecLoadTilesInfo::BBOX;

	Load(info);
}

void TileManager::LoadLocality(const Viewer& viewer, int flags) {
	RecLoadTilesInfo info;

	info.viewer = &viewer;
	info.flags = flags;
	info.mode = RecLoadTilesInfo::LOCALITY;

	Load(info);
}

bool TileManager::GenerationCompare(QuadNode** x, QuadNode** y) {
	return (*x)->generation < (*y)->generation;
}

void TileManager::GarbageCollect() {
	pthread_mutex_lock(&tiles_mutex_);
	if (total_size_ > size_limit_) {
		GCQueue gcqueue;
		gcqueue.reserve(tile_count_);

		/* collect tiles for garbage collecting */
		RecGarbageCollectTiles(&root_, gcqueue);

		/* sort by generation */
		std::sort(gcqueue.begin(), gcqueue.end(), GenerationCompare);

		for (GCQueue::iterator i = gcqueue.begin(); i != gcqueue.end() && total_size_ > size_limit_; ++i) {
			RecDestroyTiles(**i);
			delete **i;
			**i = NULL;
		}
	}

	generation_++;
	pthread_mutex_unlock(&tiles_mutex_);
}

void TileManager::Clear() {
	pthread_mutex_lock(&tiles_mutex_);
	RecDestroyTiles(&root_);
	generation_++;
	pthread_mutex_unlock(&tiles_mutex_);
}

void TileManager::SetLevel(int level) {
	level_ = level;
}

void TileManager::SetRange(float range) {
	range_ = range;
}

void TileManager::SetFlags(int flags) {
	flags_ = flags;
}

void TileManager::SetHeightEffect(bool enabled) {
	height_effect_ = enabled;
}

void TileManager::SetSizeLimit(size_t limit) {
	size_limit_ = limit;
}
