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

class Viewer;
class Tile;

class StaticQuadtree {
protected:
	struct Node {
		Node* child[4];

		Tile* tile;

		int generation;

		Node():  tile(NULL), generation(0) {
			child[0] = child[1] = child[2] = child[3] = NULL;
		}
	};

protected:
	const Projection projection_;
	Node* root_;
	int target_level_;
	int generation_;

protected:
	void DestroyNodes(Node* node);
	void RenderNodes(Node* node, const Viewer& viewer) const;
	void LoadNodes(Node* node, const BBoxi& bbox, int level = 0, int x = 0, int y = 0);
	void SweepNodes(Node* node);

protected:
	StaticQuadtree(const Projection projection);
	virtual ~StaticQuadtree();

	void SetTargetLevel(int level);

	void Render(const Viewer& viewer) const;

	virtual Tile* SpawnTile(const BBoxi& bbox) const = 0;

public:
	void RequestVisible(const BBoxi& bbox);
	void GarbageCollect();
};

#endif
