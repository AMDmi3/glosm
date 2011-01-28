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

#ifndef STATICQUADTREE_HH
#define STATICQUADTREE_HH

#include <glosm/BBox.hh>
#include <glosm/Projection.hh>

#include <pthread.h>
#include <list>
#include <map>

class Geometry;
class GeometryDatasource;
class Viewer;
class Tile;

class StaticQuadtree {
protected:
	enum TileLoadingFlags {
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
		Geometry* geometry;
		int generation;
		volatile bool loading;

		TileData();
		~TileData();
	};

protected:
	typedef std::map<TileId, TileData> TilesMap;

protected:
	const Projection projection_;
	const GeometryDatasource& datasource_;
	int target_level_;
	int generation_;

	TilesMap tiles_;
	mutable pthread_mutex_t tiles_mutex_;
	pthread_cond_t tiles_cond_;

	pthread_t loading_thread_;
	volatile bool thread_die_flag_;

protected:
	StaticQuadtree(const Projection projection, const GeometryDatasource& ds);
	virtual ~StaticQuadtree();

	virtual Tile* SpawnTile(const Geometry& geom, const BBoxi& bbox) const = 0;

	void LoadTiles(const BBoxi& bbox, bool sync, int level = 0, int x = 0, int y = 0);

	void LoadingThreadFunc();
	static void* LoadingThreadFuncWrapper(void* arg);

	void Render(const Viewer& viewer) const;

public:
	void SetTargetLevel(int level);
	void RequestVisible(const BBoxi& bbox, bool sync);
	void GarbageCollect();
};

#endif
