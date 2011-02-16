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

#ifndef TILEMANAGER_HH
#define TILEMANAGER_HH

#include <glosm/BBox.hh>
#include <glosm/Projection.hh>

#include <pthread.h>

#include <list>
#include <map>

class Geometry;
class GeometryDatasource;
class Viewer;
class Tile;

class TileManager {
public:
	enum RequestFlags {
		SYNC = 0x01,
		BLOB = 0x02,
	};

protected:
	struct TileId {
		int level;
		int x;
		int y;

		TileId(int lev, int xx, int yy);
		bool operator<(const TileId& other) const;
	};

	struct TileData {
		Tile* tile;
		int generation;

		TileData(Tile* t, int g): tile(t), generation(g) {
		}
	};

	struct TileTask {
		TileId id;
		BBoxi bbox;

		TileTask(const TileId& i, const BBoxi& b) : id(i), bbox(b) {
		}
	};

protected:
	/* TODO: use more clever bbox-based container */
	typedef std::multimap<TileId, TileData> TilesMap;
	typedef std::list<TileTask> TilesQueue;

protected:
	const Projection projection_;
	int target_level_;

	mutable pthread_mutex_t tiles_mutex_;
	/* protected by tiles_mutex_ */
	TilesMap tiles_;
	int generation_;
	/* /protected by tiles_mutex_ */

	mutable pthread_mutex_t queue_mutex_;
	pthread_cond_t queue_cond_;
	/* protected by queue_mutex_ */
	TilesQueue queue_;
	/* /protected by queue_mutex_ */

	pthread_t loading_thread_;
	volatile bool thread_die_flag_;

protected:
	TileManager(const Projection projection);
	virtual ~TileManager();

	virtual Tile* SpawnTile(const BBoxi& bbox) const = 0;

	int LoadTile(const TileId& id, const BBoxi& bbox, int flags);
	bool LoadTiles(const BBoxi& bbox, int flags, int level = 0, int x = 0, int y = 0);

	void LoadingThreadFunc();
	static void* LoadingThreadFuncWrapper(void* arg);

	void Render(const Viewer& viewer);

public:
	void SetTargetLevel(int level);
	void RequestVisible(const BBoxi& bbox, int flags);
	void GarbageCollect();
};

#endif
