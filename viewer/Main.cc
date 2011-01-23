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

#include <err.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__APPLE__)
#	include <OpenGL/gl.h>
#	include <GLUT/glut.h>
#else
#	include <GL/gl.h>
#	include <GL/glut.h>
#endif

#include <vector>
#include <map>

#include <glosm/Math.hh>

#include <glosm/MercatorProjection.hh>
#include <glosm/PreloadedXmlDatasource.hh>
#include <glosm/DefaultGeometryGenerator.hh>
#include <glosm/FirstPersonViewer.hh>
#include <glosm/GeometryLayer.hh>

/* as glut has no OO concept and no feature like setUserData,
 * please forgive me using global variables pointing to
 * stack data for now */
FirstPersonViewer viewer;
GeometryLayer* layer_p = NULL;

int screenw = 1;
int screenh = 1;
int movementflags = 0;
float speed = 200.0f;
int lockheight = 0;

struct timeval prevtime, curtime, fpstime;
int nframes = 0;

void Display(void) {
	/* update scene */
	gettimeofday(&curtime, NULL);
	float dt = (float)(curtime.tv_sec - prevtime.tv_sec) + (float)(curtime.tv_usec - prevtime.tv_usec)/1000000.0f;

	/* render frame */
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (layer_p) {
		int radius = 0;
		layer_p->RequestVisible(BBoxi(viewer.GetPos(MercatorProjection()) - Vector2i(radius, radius), viewer.GetPos(MercatorProjection()) + Vector2i(radius, radius)));
		layer_p->Render(viewer);
	}

	glFlush();
	glutSwapBuffers();

	if (movementflags) {
		float myspeed = speed;
		float height = viewer.MutablePos().z / 1000.0;

		if (height > 100.0 && height < 100000.0)
			myspeed *= height / 100.0;
		else if (height >= 100000.0)
			myspeed *= 1000.0;

		viewer.Move(movementflags, myspeed, dt);
	}
	if (lockheight != 0)
		viewer.MutablePos().z = lockheight;

	/* update FPS */
	float fpst = (float)(curtime.tv_sec - fpstime.tv_sec) + (float)(curtime.tv_usec - fpstime.tv_usec)/1000000.0f;

	if (fpst > 10.0) {
		fprintf(stderr, "FPS: %.3f\n", (float)nframes/fpst);
		fpstime = curtime;
		nframes = 0;
	}

	prevtime = curtime;
	nframes++;

	/* frame limiter */
	usleep(10000);
}

void Reshape(int w, int h) {
	if (w <= 0)
		w = 1;
	if (h <= 0)
		h = 1;

	screenw = w;
	screenh = h;

	float wanted_fov = 70.0f/180.0f*M_PI;
	float wanted_aspect = 4.0f/3.0f;

	float fov, aspect;

	if ((float)w/(float)h > wanted_aspect) { // wider than wanted
		fov = wanted_fov;
	} else { // narrower than wanted
		float wanted_h = (float)w/wanted_aspect;
		fov = 2.0f*atanf((float)h / wanted_h * tanf(wanted_fov/2.0f));
	}
	fov = wanted_fov;
	aspect = (float)w/(float)h;

	glViewport(0, 0, w, h);

	viewer.SetFov(fov);
	viewer.SetAspect(aspect);

	glutWarpPointer(screenw/2, screenh/2);
}

void Mouse(int x, int y) {
	int dx = x - screenw/2;
	int dy = y - screenh/2;

	float YawDelta = (float)dx / 500.0;
	float PitchDelta = -(float)dy / 500.0;

	viewer.HardRotate(YawDelta, PitchDelta);

	if (dx != 0 || dy != 0)
		glutWarpPointer(screenw/2, screenh/2);
}

void SpecialDown(int key, int, int) {
	switch (key) {
	case GLUT_KEY_UP: movementflags |= FirstPersonViewer::FORWARD; break;
	case GLUT_KEY_DOWN: movementflags |= FirstPersonViewer::BACKWARD; break;
	case GLUT_KEY_LEFT: movementflags |= FirstPersonViewer::LEFT; break;
	case GLUT_KEY_RIGHT: movementflags |= FirstPersonViewer::RIGHT; break;
	default:
		  break;
	}
}

void SpecialUp(int key, int, int) {
	switch (key) {
	case GLUT_KEY_UP: movementflags &= ~FirstPersonViewer::FORWARD; break;
	case GLUT_KEY_DOWN: movementflags &= ~FirstPersonViewer::BACKWARD; break;
	case GLUT_KEY_LEFT: movementflags &= ~FirstPersonViewer::LEFT; break;
	case GLUT_KEY_RIGHT: movementflags &= ~FirstPersonViewer::RIGHT; break;
	default:
		  break;
	}
}

void KeyDown(unsigned char key, int, int) {
	switch (key) {
	case 27: case 'q': exit(0); break;
	case 'w': movementflags |= FirstPersonViewer::FORWARD; break;
	case 's': movementflags |= FirstPersonViewer::BACKWARD; break;
	case 'a': movementflags |= FirstPersonViewer::LEFT; break;
	case 'd': movementflags |= FirstPersonViewer::RIGHT; break;
	case 'c': movementflags |= FirstPersonViewer::LOWER; break;
	case ' ': movementflags |= FirstPersonViewer::HIGHER; break;
	case 'l': lockheight = (lockheight == 0 ? viewer.MutablePos().z : 0); break;
	case 'h': lockheight = (lockheight == 0 ? 1750 : 0); break;
	case '+': speed *= 5.0f; break;
	case '-': speed /= 5.0f; break;
	default:
		  break;
	}
}

void KeyUp(unsigned char key, int, int) {
	switch (key) {
	case 'w': movementflags &= ~FirstPersonViewer::FORWARD; break;
	case 's': movementflags &= ~FirstPersonViewer::BACKWARD; break;
	case 'a': movementflags &= ~FirstPersonViewer::LEFT; break;
	case 'd': movementflags &= ~FirstPersonViewer::RIGHT; break;
	case 'c': movementflags &= ~FirstPersonViewer::LOWER; break;
	case ' ': movementflags &= ~FirstPersonViewer::HIGHER; break;
	default:
		  break;
	}
}

int real_main(int argc, char** argv) {
	glutInit(&argc, argv);

	if (argc != 2)
		errx(1, "Usage: %s file.osm", argv[0]);

	/* load data */
	fprintf(stderr, "Loading...\n");
	PreloadedXmlDatasource osm_datasource;
	gettimeofday(&prevtime, NULL);
	osm_datasource.Load(argv[1]);
	gettimeofday(&curtime, NULL);
	fprintf(stderr, "Loaded XML in %.3f seconds\n", (float)(curtime.tv_sec - prevtime.tv_sec) + (float)(curtime.tv_usec - prevtime.tv_usec)/1000000.0f);
	prevtime = curtime;
	fpstime = curtime;

	/* glut init */
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("glosm viewer");

	glutIgnoreKeyRepeat(1);
	glutSetCursor(GLUT_CURSOR_NONE);

	glutDisplayFunc(Display);
	glutIdleFunc(Display);
	glutReshapeFunc(Reshape);
	glutPassiveMotionFunc(Mouse);
	glutKeyboardFunc(KeyDown);
	glutKeyboardUpFunc(KeyUp);
	glutSpecialFunc(SpecialDown);
	glutSpecialUpFunc(SpecialUp);

	/* glosm init */
	DefaultGeometryGenerator geometry_generator(osm_datasource);
	GeometryLayer layer(MercatorProjection(), geometry_generator);
	layer_p = &layer;

	int height = fabs((float)geometry_generator.GetBBox().top -  (float)geometry_generator.GetBBox().bottom)/3600000000.0*40000000.0/10.0*1000.0;
	viewer.SetPos(Vector3i(geometry_generator.GetCenter(), height));

	/* main loop */
	/* note that this never returns and objects created above
	 * are never properly destroyed; should dump GLUT ASAP */
	glutMainLoop();

	return 0;
}

int main(int argc, char** argv) {
	try {
		return real_main(argc, argv);
	} catch (std::exception &e) {
		fprintf(stderr, "Exception: %s\n", e.what());
	} catch (...) {
		fprintf(stderr, "Unknown exception\n");
	}

	return 1;
}
