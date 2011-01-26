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
class Tile;
class Viewer;

class StaticQuadtree {
protected:
	struct Node {
		Node* child[4];

		Tile* tile;
		Geometry* geometry;

		int generation;

		volatile bool pending;

		Node(): tile(NULL), geometry(NULL), generation(0) {
			child[0] = child[1] = child[2] = child[3] = NULL;
		}
	};

	struct TileId {
		int level;
		int x;
		int y;

		TileId(int lev, int xx, int yy) : level(lev), x(xx), y(yy) {
		}

		bool operator<(const TileId& other) const {
			if (level < other.level) return true;
			if (level > other.level) return false;
			if (x < other.x) return true;
			if (x > other.x) return false;
			return y < other.y;
		}
	};

	typedef std::pair<TileId, Node*> LoadingTask;

protected:
	typedef std::list<LoadingTask> LoadingQueue;
	//typedef std::map<TileId, Node*> OrphanNodeMap;

protected:
	const Projection projection_;
	const GeometryDatasource& datasource_;
	Node* root_;
	int target_level_;
	int generation_;

	LoadingQueue loading_queue_;
	pthread_t loading_thread_;
	pthread_mutex_t loading_queue_mutex_;
	pthread_cond_t loading_queue_cond_;
	volatile bool loading_thread_die_;

protected:
	StaticQuadtree(const Projection projection, const GeometryDatasource& ds);
	virtual ~StaticQuadtree();

	virtual Tile* SpawnTile(const Geometry& geom, const BBoxi& bbox) const = 0;

	int DestroyNodes(Node* node);
	void RenderNodes(Node* node, const Viewer& viewer) const;
	void LoadNodes(Node* node, const BBoxi& bbox, bool sync, int level = 0, int x = 0, int y = 0);
	void SweepNodes(Node* node);

	void EnqueueTile(Node* node, int level, int x, int y);
	void CleanupQueue();

	void LoadingThreadFunc();
	static void* LoadingThreadFuncWrapper(void* arg);

	void Render(const Viewer& viewer) const;

public:
	void SetTargetLevel(int level);
	void RequestVisible(const BBoxi& bbox, bool sync);
	void GarbageCollect();
};

#endif
