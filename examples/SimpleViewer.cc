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

/*
 * This is nearly simplest possible glosm example.
 *
 * This application is basically glosm-viewer stripped of most
 * functionality to show you how glosm works.
 */

#include <glosm/Math.hh>
#include <glosm/MercatorProjection.hh>
#include <glosm/PreloadedXmlDatasource.hh>
#include <glosm/FirstPersonViewer.hh>
#include <glosm/GeometryLayer.hh>
#include <glosm/GeometryGenerator.hh>
#include <glosm/geomath.h>

#if defined(WITH_GLEW)
#	include <GL/glew.h>
#endif
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>

Viewer* g_viewer = NULL;
GeometryLayer* g_layer = NULL;

void Display(void) {
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*** glosm stuff begins ***/

	if (g_layer && g_viewer)
		g_layer->Render(*g_viewer);

	/*** glosm stuff ends ***/

	glFlush();
}

void KeyDown(unsigned char, int, int) {
	exit(0);
}

int main(int argc, char** argv) {
	/* GLUT init */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutCreateWindow("SimpleViewer");
	glutDisplayFunc(Display);
	glutKeyboardFunc(KeyDown);

#if defined WITH_GLEW
	glewInit();
#endif

	/*** glosm stuff begins ***/

	/* 1) Load our data */
	PreloadedXmlDatasource osm_datasource;
	osm_datasource.Load(TESTDATA);

	/* 2) Create facility to translate OSM data to geometry */
	GeometryGenerator geometry_generator(osm_datasource);

	/* 3) Create layer which will render that geometry */
	GeometryLayer layer(MercatorProjection(), geometry_generator);

	/* 4) Request all data to be loaded synchronously */
	layer.LoadArea(geometry_generator.GetBBox(), TileManager::SYNC);

	/* 5) Create viewer to control eye position and direction */
	FirstPersonViewer viewer;
	viewer.SetPos(Vector3i(geometry_generator.GetCenter(), 100 * GEOM_UNITSINMETER /* 100 meters */));
	viewer.SetAspect(800.0/600.0);
	viewer.SetRotation(-135.0 / 180.0 * M_PI, -60.0 / 180.0 * M_PI);

	/*** glosm stuff ends ***/

	g_viewer = &viewer;
	g_layer = &layer;

	/* main loop */
	glutMainLoop();

	return 0;
}
