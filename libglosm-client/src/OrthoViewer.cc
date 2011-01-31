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

#if defined(__APPLE__)
#	include <OpenGL/gl.h>
#else
#	include <GL/gl.h>
#endif

#include <glosm/Projection.hh>

#include <glosm/OrthoViewer.hh>

#include <glosm/geomath.h>

OrthoViewer::OrthoViewer() : bbox_(BBoxi::ForEarth()), skew_(0.0f) {
}

void OrthoViewer::SetupViewerMatrix(const Projection& projection) const {
	Vector3i center = GetPos(projection);

	float xspan = fabs(projection.Project(Vector2i(bbox_.left, 0), center).x);
	float yspan = fabs(projection.Project(Vector2i(0, bbox_.top), center).y);

	/* Take +/- 1km as Z range */
	float zspan = fabs(projection.Project(Vector3i(center.x, center.y, 1000 * GEOM_UNITSINMETER), Vector3i(center.x, center.y, 0)).z);

	/* undefined which is aspect: x/y or y/x */
	float perspective[] = {
		1.0f/xspan,  0.0f, 0.0f, 0.0f,
		0.0f, 1.0f/yspan, 0.0f, 0.0f,
		0.0f, skew_/yspan, -1.0/zspan, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(perspective);
}

Vector3i OrthoViewer::GetPos(const Projection& projection) const {
	/* `naive' center that doesn't take projection into account */
	Vector2i nearcenter = bbox_.GetCenter();

	/* real center that takes projectino into account; to minimize error, calculated relative to nearcenter */
	return projection.UnProject((projection.Project(bbox_.GetBottomLeft(), nearcenter) + projection.Project(bbox_.GetTopRight(), nearcenter))/2.0, nearcenter);
}

void OrthoViewer::SetBBox(const BBoxi& bbox) {
	bbox_ = bbox;
}

void OrthoViewer::SetBBoxForTile(int nx, int ny, int zoom) {
	bbox_.left = round(((double)nx) / pow(2.0, zoom) * GEOM_LONSPAN - GEOM_MAXLON);
	bbox_.right = round(((double)nx + 1.0) / pow(2.0, zoom) * GEOM_LONSPAN - GEOM_MAXLON);
	bbox_.bottom = round(-unmercator(((float)ny) / powf(2.0, zoom) * M_PI * 2.0 - M_PI) * GEOM_RAD_TO_DEG);
	bbox_.top = round(-unmercator(((float)ny + 1.0) / powf(2.0, zoom) * M_PI * 2.0 - M_PI) * GEOM_RAD_TO_DEG);
}

void OrthoViewer::SetSkew(float skew) {
	skew_ = skew;
}
