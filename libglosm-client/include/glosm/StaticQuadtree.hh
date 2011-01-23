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
		Node* nw;
		Node* ne;
		Node* se;
		Node* sw;

		Tile* tile;

		Node(): nw(NULL), ne(NULL), se(NULL), sw(NULL), tile(NULL) {
		}
	};

protected:
	const Projection projection_;
	Node* root_;
	int target_level_;

protected:
	void DestroyNodes(Node* node);
	void RenderNodes(Node* node, const Viewer& viewer) const;
	void LoadNodes(Node* node, const BBoxi& bbox, int level = 0, int x = 0, int y = 0);

protected:
	StaticQuadtree(const Projection projection);
	virtual ~StaticQuadtree();

	void SetTargetLevel(int level);

	void Render(const Viewer& viewer) const;

	virtual Tile* SpawnTile(const BBoxi& bbox) const = 0;

public:
	void RequestVisible(const BBoxi& bbox);
};

#endif
