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

#include <glosm/TileManager.hh>

#include <glosm/Viewer.hh>
#include <glosm/Geometry.hh>
#include <glosm/GeometryDatasource.hh>
#include <glosm/Tile.hh>
#include <glosm/Exception.hh>

#if defined(__APPLE__)
#	include <OpenGL/gl.h>
#else
#	include <GL/gl.h>
#endif

#include <cassert>

TileManager::TileId::TileId(int lev, int xx, int yy) : level(lev), x(xx), y(yy) {
}

bool TileManager::TileId::operator<(const TileId& other) const {
	if (level < other.level) return true;
	if (level > other.level) return false;
	if (x < other.x) return true;
	if (x > other.x) return false;
	return y < other.y;
}

TileManager::TileManager(const Projection projection): projection_(projection) {
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

	target_level_ = 0;
	generation_ = 0;
	thread_die_flag_ = false;
}

TileManager::~TileManager() {
	thread_die_flag_ = true;
	pthread_cond_signal(&queue_cond_);

	pthread_join(loading_thread_, NULL);

	pthread_cond_destroy(&queue_cond_);
	pthread_mutex_destroy(&queue_mutex_);
	pthread_mutex_destroy(&tiles_mutex_);

	for (TilesMap::iterator i = tiles_.begin(); i != tiles_.end(); ++i)
		delete i->second.tile;
}

int TileManager::LoadTile(const TileId& id, const BBoxi& bbox, int flags) {
	int ret = 0;
	if (flags & SYNC) {
		Tile* tile = SpawnTile(bbox);
		/* TODO: we may call tile->BindBuffers here */
		tiles_.insert(std::make_pair(id, TileData(tile, generation_)));
	} else {
		bool added = false;
		pthread_mutex_lock(&queue_mutex_);
		/* don't needlessly enqueue more tiles that we can process in a frame time*/
		/* TODO: this should be user-settable, as we don't necessarily do GC/loading
		 * every frame */
		if (loading_.find(id) == loading_.end() && queue.size() < 2) {
			added = true;
			queue_.push_back(TileTask(id, bbox));
		}
		ret = queue_.size();
		pthread_mutex_unlock(&queue_mutex_);
		if (added)
			pthread_cond_signal(&queue_cond_);
	}

	return ret;
}

/*
 * recursive quadtree processing
 */

bool TileManager::LoadTiles(const BBoxi& bbox, int flags, int level, int x, int y) {
	if (level == target_level_) {
		TilesMap::iterator thistile = tiles_.find(TileId(level, x, y));

		if (thistile != tiles_.end()) {
			thistile->second.generation = generation_;
			return true; /* tile already loaded */
		}

		BBoxi bbox = BBoxi::ForGeoTile(level, x, y);

		LoadTile(TileId(level, x, y), BBoxi::ForGeoTile(level, x, y), flags);

		/* no deeper recursion */
		return true;
	}

	/* children */
	for (int d = 0; d < 4; ++d) {
		int xx = x * 2 + d % 2;
		int yy = y * 2 + d / 2;
		if (BBoxi::ForGeoTile(level + 1, xx, yy).Intersects(bbox)) {
			if (!LoadTiles(bbox, flags, level + 1, xx, yy))
				return false;
		}
	}

	return true;
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
		std::pair<LoadingSet::iterator, bool> pair = loading_.insert(task.id);
		assert(pair.second);

		pthread_mutex_unlock(&queue_mutex_);

		/* load tile */
		Tile* tile = SpawnTile(task.bbox);

		pthread_mutex_lock(&tiles_mutex_);
		tiles_.insert(std::make_pair(task.id, TileData(tile, generation_)));
		pthread_mutex_unlock(&tiles_mutex_);

		pthread_mutex_lock(&queue_mutex_);
		loading_.erase(pair.first);
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

	/* and render them */
	for (TilesMap::iterator i = tiles_.begin(); i != tiles_.end(); ++i) {
		if (i->second.generation != generation_)
			continue;

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		/* prepare modelview matrix for the tile: position
		 * it in the right place given that viewer is always
		 * at (0, 0, 0) */
		Vector3f offset = projection_.Project(i->second.tile->GetReference(), Vector2i(viewer.GetPos(projection_))) +
				projection_.Project(Vector2i(viewer.GetPos(projection_)), viewer.GetPos(projection_));

		glTranslatef(offset.x, offset.y, offset.z);

		/* same for rotation */
		Vector3i ref = i->second.tile->GetReference();
		Vector3i pos = viewer.GetPos(projection_);

		/* normal at tile's reference point */
		Vector3d refnormal = (
				(Vector3d)projection_.Project(Vector3i(ref.x, ref.y, std::numeric_limits<osmint_t>::max()), pos) -
				(Vector3d)projection_.Project(Vector3i(ref.x, ref.y, 0), pos)
			).Normalized();

		/* normal at reference point projected to equator */
		Vector3d refeqnormal = (
				(Vector3d)projection_.Project(Vector3i(ref.x, 0, std::numeric_limits<osmint_t>::max()), pos) -
				(Vector3d)projection_.Project(Vector3i(ref.x, 0, 0), pos)
			).Normalized();

		/* normal at north pole */
		Vector3d polenormal = (
				(Vector3d)projection_.Project(Vector3i(ref.x, 900000000, std::numeric_limits<osmint_t>::max()), pos) -
				(Vector3d)projection_.Project(Vector3i(ref.x, 900000000, 0), pos)
			).Normalized();

		/* XXX: IsValid() check basically detects
		 * MercatorProjection and does no rotation for it.
		 * While is's ok for now, this may need more generic
		 * approach in future */
		if (polenormal.IsValid()) {
			Vector3d side = refnormal.CrossProduct(polenormal).Normalized();

			glRotatef((double)((osmlong_t)ref.y - (osmlong_t)pos.y) / 10000000.0, side.x, side.y, side.z);
			glRotatef((double)((osmlong_t)ref.x - (osmlong_t)pos.x) / 10000000.0, polenormal.x, polenormal.y, polenormal.z);
		}

		i->second.tile->Render();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	pthread_mutex_unlock(&tiles_mutex_);
}

/*
 * public interface
 */

void TileManager::SetTargetLevel(int level) {
	target_level_ = level;
}

void TileManager::RequestVisible(const BBoxi& bbox, int flags) {
	if (!flags & SYNC) {
		pthread_mutex_lock(&queue_mutex_);
		queue_.clear();
		pthread_mutex_unlock(&queue_mutex_);
	}

	pthread_mutex_lock(&tiles_mutex_);
	if (flags & BLOB)
		LoadTile(TileId(0, 0, 0), bbox, flags);
	else
		LoadTiles(bbox, flags);
	pthread_mutex_unlock(&tiles_mutex_);
}

void TileManager::GarbageCollect() {
	/* TODO: may put deletable tiles into list and delete after
	 * unlocking mutex for less contention */
	pthread_mutex_lock(&tiles_mutex_);
	for (TilesMap::iterator i = tiles_.begin(); i != tiles_.end(); ) {
		if (i->second.generation != generation_) {
			delete i->second.tile;
			TilesMap::iterator tmp = i++;
			tiles_.erase(tmp);
		} else {
			i++;
		}
	}
	generation_++;
	pthread_mutex_unlock(&tiles_mutex_);
}
