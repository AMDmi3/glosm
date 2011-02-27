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

/**
 * Generic quadtree tile manager
 *
 * This class is serves as a base class for layers and manages tile
 * loading, displaying and disposal.
 *
 * @todo this class is planned to handle multilayer tile hierarchies,
 * however for now level is fixed
 */
class TileManager {
public:
	enum RequestFlags {
		SYNC = 0x01,
		BLOB = 0x02,
	};

protected:
	/**
	 * Tile identifier
	 */
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

	/**
	 * Single node of a quadtree
	 */
	struct QuadNode {
		Tile* tile;
		int generation;
		BBoxi bbox;

		QuadNode* childs[4];

		QuadNode() : tile(NULL), generation(0), bbox(BBoxi::ForGeoTile(0, 0, 0)) {
			childs[0] = childs[1] = childs[2] = childs[3] = NULL;
		}
	};

	/**
	 * Single tile loading request
	 */
	struct TileTask {
		TileId id;
		BBoxi bbox;

		TileTask(const TileId& i, const BBoxi& b) : id(i), bbox(b) {
		}
	};

	/**
	 * Holder of data for LoadLocality request
	 */
	struct RecLoadTilesInfo {
		enum Modes {
			BBOX,
			LOCALITY
		};

		union {
			const Viewer* viewer;
			const BBoxi* bbox;
		};

		int mode;
		int flags;
		Vector3i viewer_pos;
		float closest_distance;
		int queue_size;

		RecLoadTilesInfo() : queue_size(0) {
		}
	};

protected:
	typedef std::list<TileTask> TilesQueue;
	typedef std::vector<QuadNode**> GCQueue;

protected:
	/* @todo it would be optimal to delegate these to layer via either
	 * virtual methods or templates */
	int level_;
	float range_;
	volatile int flags_;
	bool height_effect_;
	size_t size_limit_;

	const Projection projection_;

	mutable pthread_mutex_t tiles_mutex_;
	/* protected by tiles_mutex_ */
	QuadNode root_;
	int generation_;
	size_t total_size_;
	int tile_count_;
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
	/**
	 * Constructs Tile
	 */
	TileManager(const Projection projection);

	/**
	 * Destructor
	 */
	virtual ~TileManager();

	/**
	 * Spawns a single tile with specified bbox
	 */
	virtual Tile* SpawnTile(const BBoxi& bbox, int flags) const = 0;

	/**
	 * Recursive tile loading function for viewer's locality
	 *
	 * @todo remove code duplication with RecLoadTilesBBox
	 */
	void RecLoadTilesLocality(RecLoadTilesInfo& info, QuadNode** pnode, int level = 0, int x = 0, int y = 0);

	/**
	 * Recursive tile loading function for given bbox
	 *
	 * @todo remove code duplication with RecLoadTilesLocality
	 */
	void RecLoadTilesBBox(RecLoadTilesInfo& info, QuadNode** pnode, int level = 0, int x = 0, int y = 0);

	/**
	 * Recursive function that places tile into specified quadtree point
	 */
	void RecPlaceTile(QuadNode* node, Tile* tile, int level = 0, int x = 0, int y = 0);

	/**
	 * Recursive function for tile rendering
	 */
	void RecRenderTiles(QuadNode* node, const Viewer& viewer);

	/**
	 * Recursive function for destroying tiles and quadtree nodes
	 */
	void RecDestroyTiles(QuadNode* node);

	/**
	 * Recursive function for garbage collecting unneeded tiles
	 */
	void RecGarbageCollectTiles(QuadNode* node, GCQueue& gcqueue);

	/**
	 * Thread function for tile loading
	 */
	void LoadingThreadFunc();

	/**
	 * Static wrapper for thread function
	 */
	static void* LoadingThreadFuncWrapper(void* arg);

	/**
	 * Loads tiles
	 */
	void Load(RecLoadTilesInfo& info);

protected:
	/**
	 * Renders visible tiles
	 */
	void Render(const Viewer& viewer);

protected:
	static bool GenerationCompare(QuadNode** x, QuadNode** y);

public:
	/**
	 * Loads square area of tiles
	 */
	void LoadArea(const BBoxi& bbox, int flags = 0);

	/**
	 * Loads tiles in locality of Viewer
	 */
	void LoadLocality(const Viewer& viewer, int flags = 0);

	/**
	 * Destroys unneeded tiles
	 */
	void GarbageCollect();

	/**
	 * Destroys all tiles
	 */
	void Clear();

	/**
	 * Sets designated tile level
	 *
	 * @param level desired level
	 */
	void SetLevel(int level);

	/**
	 * Sets range in which tiles are visible
	 *
	 * @param range range in meters
	 */
	void SetRange(float range);

	/**
	 * Sets flags for tile spawinig
	 *
	 * @param flags flags
	 * @see GeometryGenerator::GetGeometry
	 */
	void SetFlags(int flags);

	/**
	 * Sets mode of taking viewer height into account
	 *
	 * @param enabled if true, height is taken into account
	 *        when calculating distance from viewer to tile
	 */
	void SetHeightEffect(bool enabled);

	/**
	 * Sets limit on cumulative tiles size
	 *
	 * @param limit size limit in bytes
	 */
	void SetSizeLimit(size_t limit);
};

#endif
