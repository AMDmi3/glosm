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

	if (node->nw)
		DestroyNodes(node->nw);
	if (node->ne)
		DestroyNodes(node->ne);
	if (node->se)
		DestroyNodes(node->se);
	if (node->sw)
		DestroyNodes(node->sw);

	delete node;
}

void StaticQuadtree::RenderNodes(Node* node, const Viewer& viewer) const {
	if (node->tile) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		Vector3f offset = projection_.Project(node->tile->GetReference(), viewer.GetPos(projection_));
		glTranslatef(offset.x, offset.y, offset.z);

		node->tile->Render();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	if (node->nw)
		RenderNodes(node->nw, viewer);
	if (node->ne)
		RenderNodes(node->ne, viewer);
	if (node->se)
		RenderNodes(node->se, viewer);
	if (node->sw)
		RenderNodes(node->sw, viewer);
}

void StaticQuadtree::LoadNodes(Node* node, const BBoxi& bbox, int level, int x, int y) {
	BBoxi thisbbox = BBoxi::ForGeoTile(level, x, y);
	if (!bbox.Intersects(thisbbox))
		return;

	if (level == target_level_) {
		if (node->tile == NULL)
			node->tile = SpawnTile(thisbbox);
	} else {
		if (!node->nw)
			node->nw = new Node;
		if (!node->ne)
			node->ne = new Node;
		if (!node->se)
			node->se = new Node;
		if (!node->sw)
			node->sw = new Node;

		LoadNodes(node->nw, bbox, level + 1, x * 2, y * 2);
		LoadNodes(node->ne, bbox, level + 1, x * 2 + 1, y * 2);
		LoadNodes(node->se, bbox, level + 1, x * 2 + 1, y * 2 + 1);
		LoadNodes(node->sw, bbox, level + 1, x * 2, y * 2 + 1);
	}
}

StaticQuadtree::StaticQuadtree(const Projection projection): projection_(projection), root_(new Node()), target_level_(16) {
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
	LoadNodes(root_, bbox);
}
