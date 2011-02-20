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

#include "GlosmViewer.hh"

#include <glosm/Math.hh>
#include <glosm/MercatorProjection.hh>
#include <glosm/SphericalProjection.hh>
#include <glosm/geomath.h>

#include <getopt.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(WITH_GLEW)
#   include <GL/glew.h>
#endif

#if defined(__APPLE__)
#	include <OpenGL/gl.h>
#else
#	include <GL/gl.h>
#endif

#include <cstdio>

GlosmViewer::GlosmViewer() : projection_(MercatorProjection()), viewer_(new FirstPersonViewer) {
	tile_level_ = -1;
	screenw_ = screenh_ = 1;
	nframes_ = 0;

	movementflags_ = 0;
	speed_ = 200.0f;
	lockheight_ = 0;

	mouse_capture_ = drag_ = slow_ = fast_ = false;

#if defined(WITH_TOUCHPAD)
	mouse_capture_ = true;
#endif
}

void GlosmViewer::Usage(const char* progname) {
	fprintf(stderr, "Usage: %s [-s] file.osm\n", progname);
	exit(1);
}

void GlosmViewer::Init(int argc, char** argv) {
	/* argument parsing */
	int c;
	const char* progname = argv[0];
	while ((c = getopt(argc, argv, "st:")) != -1) {
		switch (c) {
		case 's': projection_ = SphericalProjection(); break;
		case 't': tile_level_ = strtol(optarg, NULL, 10); break;
		default:
			Usage(progname);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1)
		Usage(progname);

	/* load data */
	fprintf(stderr, "Loading...\n");
	gettimeofday(&prevtime_, NULL);

	osm_datasource_.reset(new PreloadedXmlDatasource);

	osm_datasource_->Load(argv[0]);
	gettimeofday(&curtime_, NULL);

	fprintf(stderr, "Loaded XML in %.3f seconds\n", (float)(curtime_.tv_sec - prevtime_.tv_sec) + (float)(curtime_.tv_usec - prevtime_.tv_usec)/1000000.0f);
	prevtime_ = curtime_;
	fpstime_ = curtime_;
}

void GlosmViewer::InitGL() {
	ShowCursor(!mouse_capture_);

#if defined(WITH_GLEW)
	GLenum err = glewInit();
	if (err != GLEW_OK)
		throw Exception() << "Cannot init glew: " << glewGetErrorString(err);
	const char *gl_requirements = "GL_VERSION_1_5";
	if (!glewIsSupported(gl_requirements))
		throw Exception() << "Minimal OpenGL requirements (" << gl_requirements << ") not met, unable to continue";
#endif

	geometry_generator_.reset(new GeometryGenerator(*osm_datasource_));
	geometry_layer_.reset(new GeometryLayer(projection_, *geometry_generator_));

	if (tile_level_ >= 0)
		geometry_layer_->SetTargetLevel(tile_level_);
	else
		geometry_layer_->RequestVisible(geometry_generator_->GetBBox(), TileManager::EXPLICIT);

	int height = fabs((float)geometry_generator_->GetBBox().top - (float)geometry_generator_->GetBBox().bottom) / GEOM_LONSPAN * WGS84_EARTH_EQ_LENGTH * GEOM_UNITSINMETER / 10.0;
	viewer_->SetPos(Vector3i(geometry_generator_->GetCenter(), height));
#if defined(WITH_TOUCHPAD)
	lockheight_ = height;
#endif
	viewer_->SetRotation(0, -M_PI_4);
}

void GlosmViewer::Render() {
	/* update scene */
	gettimeofday(&curtime_, NULL);
	float dt = (float)(curtime_.tv_sec - prevtime_.tv_sec) + (float)(curtime_.tv_usec - prevtime_.tv_usec)/1000000.0f;

	/* render frame */
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (tile_level_ >= 0) {
		int radius = 5000000;
		geometry_layer_->RequestVisible(BBoxi(viewer_->GetPos(MercatorProjection()) - Vector2i(radius, radius), viewer_->GetPos(MercatorProjection()) + Vector2i(radius, radius)), 0);
		geometry_layer_->GarbageCollect();
	}
	geometry_layer_->Render(*viewer_);

	glFlush();

	Flip();

	/* movement */
	if (movementflags_) {
		float myspeed = speed_;
		float height = viewer_->MutablePos().z / GEOM_UNITSINMETER;

		/* don't scale down under 100 meters */
		if (height > 100.0)
			myspeed *= height / 100.0;

		if (fast_)
			myspeed *= 5.0;
		if (slow_)
			myspeed /= 5.0;

		viewer_->Move(movementflags_, myspeed, dt);
	}
	if (lockheight_ != 0)
		viewer_->MutablePos().z = lockheight_;

	/* update FPS */
	float fpst = (float)(curtime_.tv_sec - fpstime_.tv_sec) + (float)(curtime_.tv_usec - fpstime_.tv_usec)/1000000.0f;

	if (fpst > 10.0) {
		fprintf(stderr, "FPS: %.3f\n", (float)nframes_/fpst);
		fpstime_ = curtime_;
		nframes_ = 0;
	}

	prevtime_ = curtime_;
	nframes_++;

	/* frame limiter */
	usleep(10000);
}

void GlosmViewer::Resize(int w, int h) {
	if (w <= 0)
		w = 1;
	if (h <= 0)
		h = 1;

	screenw_ = w;
	screenh_ = h;

	float wanted_fov = 70.0f/180.0f*M_PI;
	float wanted_aspect = 4.0f/3.0f;

	float fov, aspect;

	if ((float)w/(float)h > wanted_aspect) { // wider than wanted
		fov = wanted_fov;
	} else { // narrower than wanted
		float wanted_h = (float)w / wanted_aspect;
		fov = 2.0f * atanf((float)h / wanted_h * tanf(wanted_fov/2.0f));
	}
	fov = wanted_fov;
	aspect = (float)w/(float)h;

	glViewport(0, 0, w, h);

	viewer_->SetFov(fov);
	viewer_->SetAspect(aspect);

	WarpCursor(w/2, h/2);
}

void GlosmViewer::KeyDown(int key) {
	switch (key) {
	case 27: case 'q':
		exit(0);
		break;
	case 'w': case KEY_UP:
		movementflags_ |= FirstPersonViewer::FORWARD;
		break;
	case 's': case KEY_DOWN:
		movementflags_ |= FirstPersonViewer::BACKWARD;
		break;
	case 'a': case KEY_LEFT:
		movementflags_ |= FirstPersonViewer::LEFT;
		break;
	case 'd': case KEY_RIGHT:
		movementflags_ |= FirstPersonViewer::RIGHT;
		break;
	case 'c':
		movementflags_ |= FirstPersonViewer::LOWER;
		break;
	case ' ':
		movementflags_ |= FirstPersonViewer::HIGHER;
		break;
	case 'l':
		lockheight_ = (lockheight_ == 0 ? viewer_->MutablePos().z : 0);
		break;
	case 'h':
		lockheight_ = (lockheight_ == 0 ? 1750 : 0);
		break;
	case '+':
		speed_ *= 5.0f;
		break;
	case '-':
		speed_ /= 5.0f;
		break;
	case KEY_SHIFT:
		fast_ = true;
		break;
	case KEY_CTRL:
		slow_ = true;
		break;
	default:
		break;
	}
}

void GlosmViewer::KeyUp(int key) {
	switch (key) {
	case 'w': case KEY_UP:
		movementflags_ &= ~FirstPersonViewer::FORWARD;
		break;
	case 's': case KEY_DOWN:
		movementflags_ &= ~FirstPersonViewer::BACKWARD;
		break;
	case 'a': case KEY_LEFT:
		movementflags_ &= ~FirstPersonViewer::LEFT;
		break;
	case 'd': case KEY_RIGHT:
		movementflags_ &= ~FirstPersonViewer::RIGHT;
		break;
	case 'c':
		movementflags_ &= ~FirstPersonViewer::LOWER;
		break;
	case ' ':
		movementflags_ &= ~FirstPersonViewer::HIGHER;
		break;
	case KEY_SHIFT:
		fast_ = false;
		break;
	case KEY_CTRL:
		slow_ = false;
		break;
	default:
		break;
	}
}

void GlosmViewer::MouseMove(int x, int y) {
	if (drag_) {
		int dx = x - drag_start_pos_.x;
		int dy = y - drag_start_pos_.y;

		float yawdelta = -(float)dx / (float)screenw_ * viewer_->GetFov() * viewer_->GetAspect();
		float pitchdelta = (float)dy / (float)screenh_ * viewer_->GetFov();

		viewer_->SetRotation(drag_start_yaw_ + yawdelta, drag_start_pitch_ + pitchdelta);
	}

	if (mouse_capture_) {
		int dx = x - screenw_/2;
		int dy = y - screenh_/2;

		float yawdelta = (float)dx / 500.0;
		float pitchdelta = -(float)dy / 500.0;

		viewer_->Rotate(yawdelta, pitchdelta, 1.0);

		if (dx != 0 || dy != 0)
			WarpCursor(screenw_/2, screenh_/2);
	}
}

void GlosmViewer::MouseButton(int button, bool pressed, int x, int y) {
	if (button == BUTTON_RIGHT && pressed) {
		mouse_capture_ = !mouse_capture_;
		ShowCursor(!mouse_capture_);
	}

	if (!mouse_capture_ && button == BUTTON_LEFT) {
		drag_ = pressed;
		if (pressed) {
			drag_start_pos_.x = x;
			drag_start_pos_.y = y;
			drag_start_yaw_ = viewer_->GetYaw();
			drag_start_pitch_ = viewer_->GetPitch();
		}
	}
}
