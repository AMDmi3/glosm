/*
 * Copyright (C) 2010-2012 Dmitry Marakasov
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

#ifndef MGLU_H
#define MGLU_H

#include "mglu.h"

#include <glosm/Math.hh>

#include <glosm/util/gl.h>

void mglFrustum(float left, float right, float bottom, float top, float znear, float zfar) {
	float matrix[16] = {
		2.0f * znear / (right - left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f * znear / (top - bottom), 0.0f, 0.0f,
		(right + left) / (right - left), (top + bottom) / (top - bottom), - (zfar - znear) / (zfar - znear), -1.0f,
		0.0f, 0.0f, -2.0f * znear * zfar / (zfar - znear), 0.0f,
	};

	glLoadMatrixf(matrix);
}

void mgluPerspective(float fovy, float aspect, float znear, float zfar) {
	float xmin, xmax, ymin, ymax;

	ymax = znear * tan(fovy * M_PI / 360.0f);
	ymin = -ymax;
	xmax = ymax * aspect;
	xmin = -xmax;

	mglFrustum(xmin, xmax, ymin, ymax, znear, zfar);
}

void mgluLookAt(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz) {
	Vector3f forward = (Vector3f(centerx, centery, centerz) - Vector3f(eyex, eyey, eyez)).Normalized();
	Vector3f side = forward.CrossProduct(Vector3f(upx, upy, upz)).Normalized();
	Vector3f up = side.CrossProduct(forward);

	float matrix[16] = {
		side.x, up.x, -forward.x, 0.0f,
		side.y, up.y, -forward.y, 0.0f,
		side.z, up.z, -forward.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	glLoadMatrixf(matrix);
	glTranslatef(-eyex, -eyey, -eyez);
}

#endif
