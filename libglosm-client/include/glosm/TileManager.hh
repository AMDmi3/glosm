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
#include <set>

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

		TileId(int lev, int xx, int yy) : level(lev), x(xx), y(yy) {
		}

		inline bool operator==(const TileId& other) const {
			return x == other.x && y == other.y && level == other.level;
		}

		inline bool operator!=(const TileId& other) const {
			return x != other.x || y != other.y || level != other.level;
		}
	};

	struct QuadNode {
		Tile* tile;
		int generation;
		BBoxi bbox;

		QuadNode* childs[4];

		QuadNode() : tile(NULL), generation(0), bbox(BBoxi::ForGeoTile(0, 0, 0)) {
			childs[0] = childs[1] = childs[2] = childs[3] = NULL;
		}
	};

	struct TileTask {
		TileId id;
		BBoxi bbox;

		TileTask(const TileId& i, const BBoxi& b) : id(i), bbox(b) {
		}
	};

	struct RecLoadTilesInfo {
		const Viewer& viewer;
		int flags;
		Vector3i viewer_pos;
		float closest_distance;
		int queue_size;

		RecLoadTilesInfo(const Viewer& v, int f) : viewer(v), flags(f), queue_size(0) {
		}
	};

protected:
	/* TODO: use more clever bbox-based container */
	typedef std::list<TileTask> TilesQueue;

protected:
	/* TODO: these is only actual for GeometryLayer so should be
	 * delegated to layer via pure virtual function or template
	 * atg */
	int level_;
	float range_;
	int flags_;
	bool height_effect_;

	const Projection projection_;

	mutable pthread_mutex_t tiles_mutex_;
	/* protected by tiles_mutex_ */
	QuadNode root_;
	int generation_;
	/* /protected by tiles_mutex_ */

	mutable pthread_mutex_t queue_mutex_;
	pthread_cond_t queue_cond_;
	/* protected by queue_mutex_ */
	TilesQueue queue_;
	TileId loading_;
	/* /protected by queue_mutex_ */

	pthread_t loading_thread_;
	volatile bool thread_die_flag_;

protected:
	TileManager(const Projection projection);
	virtual ~TileManager();

	virtual Tile* SpawnTile(const BBoxi& bbox, int flags) const = 0;

	void RecLoadTiles(RecLoadTilesInfo& info, QuadNode** pnode, int level = 0, int x = 0, int y = 0);
	void RecPlaceTile(QuadNode* node, Tile* tile, int level = 0, int x = 0, int y = 0);
	void RecRenderTiles(QuadNode* node, const Viewer& viewer);
	void RecDestroyTiles(QuadNode* node);
	void RecGarbageCollectTiles(QuadNode* node);

	void LoadingThreadFunc();
	static void* LoadingThreadFuncWrapper(void* arg);

	void Render(const Viewer& viewer);

public:
	void LoadArea(const BBoxi& bbox, int flags = 0);
	void LoadLocality(const Viewer& viewer, int flags = 0);
	void GarbageCollect();

	void SetLevel(int level);
	void SetRange(float range);
	void SetFlags(int flags);
	void SetHeightEffect(bool enabled);
};

#endif
