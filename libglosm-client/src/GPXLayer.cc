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

#include <glosm/GPXLayer.hh>

#include <glosm/GPXDatasource.hh>
#include <glosm/HeightmapDatasource.hh>
#include <glosm/GPXTile.hh>
#include <glosm/Projection.hh>
#include <glosm/Viewer.hh>

#include <glosm/util/gl.h>

GPXLayer::GPXLayer(const Projection projection, const GPXDatasource& datasource, const HeightmapDatasource& heightmap): TileManager(projection), projection_(projection), datasource_(datasource), heightmap_(heightmap) {
}

GPXLayer::~GPXLayer() {
}

void GPXLayer::Render(const Viewer& viewer) {
	/* Setup projection */
	viewer.SetupViewerMatrix(projection_);

	/* OpenGL attrs */
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Lighting */
	GLfloat global_ambient[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat light_position[] = {-0.2, -0.777, 0.63, 0.0};
	GLfloat light_diffuse[] = {0.45, 0.45, 0.45, 1.0};
	GLfloat light_ambient[] = {0.33, 0.33, 0.33, 1.0};
	GLfloat material_diffuse[] = {1.0, 1.0, 1.0, 0.9};

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

	glMaterialfv(GL_FRONT, GL_AMBIENT, material_diffuse);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);

	/* Render tile(s) */
	TileManager::Render(viewer);
}

Tile* GPXLayer::SpawnTile(const BBoxi& bbox, int flags) const {
	return new GPXTile(projection_, datasource_, heightmap_, bbox.GetCenter(), bbox);
}
