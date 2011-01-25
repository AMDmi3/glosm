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

#include <glosm/StaticQuadtree.hh>

#if defined(__APPLE__)
#	include <OpenGL/gl.h>
#else
#	include <GL/gl.h>
#endif

#include <glosm/Viewer.hh>
#include <glosm/Tile.hh>

void StaticQuadtree::DestroyNodes(Node* node) {
	delete node->tile;

	for (int d = 0; d < 4; ++d)
		if (node->child[d])
			DestroyNodes(node->child[d]);

	delete node;
}

void StaticQuadtree::RenderNodes(Node* node, const Viewer& viewer) const {
	if (node->generation != generation_)
		return;

	if (node->tile) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		Vector3f offset = projection_.Project(node->tile->GetReference(), viewer.GetPos(projection_));
		glTranslatef(offset.x, offset.y, offset.z);

		node->tile->Render();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		return;
	}

	for (int d = 0; d < 4; ++d)
		if (node->child[d])
			RenderNodes(node->child[d], viewer);
}

void StaticQuadtree::LoadNodes(Node* node, const BBoxi& bbox, int level, int x, int y) {
	node->generation = generation_;

	if (level == target_level_) {
		/* leaf */
		if (node->tile == NULL)
			node->tile = SpawnTile(BBoxi::ForGeoTile(level, x, y));
		return;
	}

	/* children */
	for (int d = 0; d < 4; ++d) {
		int xx = x * 2 + d % 2;
		int yy = y * 2 + d / 2;
		if (BBoxi::ForGeoTile(level + 1, xx, yy).Intersects(bbox)) {
			if (!node->child[d])
				node->child[d] = new Node;
			LoadNodes(node->child[d], bbox, level + 1, xx, yy);
		}
	}
}

void StaticQuadtree::SweepNodes(Node* node) {
	for (int d = 0; d < 4; ++d) {
		if (node->child[d]) {
			if (node->child[d]->generation != generation_) {
				DestroyNodes(node->child[d]);
				node->child[d] = NULL;
			} else {
				SweepNodes(node->child[d]);
			}
		}
	}
}

StaticQuadtree::StaticQuadtree(const Projection projection): projection_(projection), root_(new Node()), target_level_(16), generation_(0) {
}

StaticQuadtree::~StaticQuadtree() {
	DestroyNodes(root_);
}

void StaticQuadtree::SetTargetLevel(int level) {
	target_level_ = level;
}

void StaticQuadtree::Render(const Viewer& viewer) const {
	RenderNodes(root_, viewer);
}

void StaticQuadtree::RequestVisible(const BBoxi& bbox) {
	++generation_;
	LoadNodes(root_, bbox);
}

void StaticQuadtree::GarbageCollect() {
	SweepNodes(root_);
}
