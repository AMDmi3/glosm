/*
 * Copyright (C) 2010-2011 Dmitry Marakasov
 *
 * This file is part of glosm.
 *
 * glosm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * glosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with glosm.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <glosm/TerrainLayer.hh>

#include <glosm/HeightmapDatasource.hh>
#include <glosm/TerrainTile.hh>
#include <glosm/Projection.hh>
#include <glosm/Viewer.hh>

#include <glosm/util/gl.h>

TerrainLayer::TerrainLayer(const Projection projection, HeightmapDatasource& datasource): TileManager(projection), projection_(projection), datasource_(datasource) {
}

TerrainLayer::~TerrainLayer() {
}

void TerrainLayer::Render(const Viewer& viewer) {
	/* Setup projection */
	viewer.SetupViewerMatrix(projection_);

	/* OpenGL attrs */
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Lighting */
	GLfloat global_ambient[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat light_position[] = {-0.2, -0.777, 0.63, 0.0};
	GLfloat light_diffuse[] = {0.45, 0.45, 0.45, 1.0};
	GLfloat light_ambient[] = {0.33, 0.33, 0.33, 1.0};
	float l = 0.85;
	GLfloat material_diffuse[] = {l, l, l, 1.0};

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

	glMaterialfv(GL_FRONT, GL_AMBIENT, material_diffuse);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/* Render tile(s) */

	/* XXX: we use 2 here as 1 is used for geometry */
	glPolygonOffset(2.0, 2.0);
	glEnable(GL_POLYGON_OFFSET_FILL);

	TileManager::Render(viewer);

	glDisable(GL_POLYGON_OFFSET_FILL);

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
}

Tile* TerrainLayer::SpawnTile(const BBoxi& bbox, int flags) const {
	return new TerrainTile(projection_, datasource_, bbox.GetCenter(), bbox);
}
