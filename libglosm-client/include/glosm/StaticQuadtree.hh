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
#include <queue>

class Geometry;
class GeometryDatasource;
class Tile;
class Viewer;

class StaticQuadtree {
protected:
	struct Node {
		Node* child[4];

		Tile* tile;

		int generation;
		volatile int queued;

		Node(): tile(NULL), generation(0) {
			child[0] = child[1] = child[2] = child[3] = NULL;
		}
	};

	struct LoadingTask {
		int level;
		int x;
		int y;

		Node* node;

		LoadingTask(int l, int xx, int yy, Node* n): level(l), x(xx), y(yy), node(n) {
		}
	};

protected:
	const Projection projection_;
	const GeometryDatasource& datasource_;
	Node* root_;
	int target_level_;
	int generation_;

	std::queue<LoadingTask> loading_queue_;
	pthread_t loading_thread_;
	pthread_mutex_t loading_queue_mutex_;
	pthread_cond_t loading_queue_cond_;
	volatile bool loading_thread_die_;

protected:
	StaticQuadtree(const Projection projection, const GeometryDatasource& ds);
	virtual ~StaticQuadtree();

	virtual Tile* SpawnTile(const Geometry& geom, const BBoxi& bbox) const = 0;

	void DestroyNodes(Node* node);
	void RenderNodes(Node* node, const Viewer& viewer) const;
	void LoadNodes(Node* node, const BBoxi& bbox, int level = 0, int x = 0, int y = 0);
	void SweepNodes(Node* node);

	void EnqueueTile(Node* node, int level, int x, int y);

	void LoadingThreadFunc();
	static void* LoadingThreadFuncWrapper(void* arg);

	void Render(const Viewer& viewer) const;

public:
	void SetTargetLevel(int level);
	void RequestVisible(const BBoxi& bbox);
	void GarbageCollect();
};

#endif
