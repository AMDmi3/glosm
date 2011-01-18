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

#include <glosm/Projection.hh>
#include <glosm/Math.hh>

#include <GL/gl.h>

TestTile::TestTile(const Projection& projection, const Vector2i& ref, const BBoxi& bbox) : Tile(ref) {
	data_[0] = projection.Project(Vector3i(ref) + Vector3i(-100, 100), ref);
	data_[1] = projection.Project(Vector3i(ref) + Vector3i(100, 100), ref);
	data_[2] = projection.Project(Vector3i(ref) + Vector3i(100, -100), ref);
	data_[3] = projection.Project(Vector3i(ref) + Vector3i(-100, -100), ref);
}

void TestTile::Render() {
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(data_[0].x, data_[0].y, data_[0].z);
	glVertex3f(data_[1].x, data_[1].y, data_[1].z);
	glVertex3f(data_[2].x, data_[2].y, data_[2].z);
	glVertex3f(data_[3].x, data_[3].y, data_[3].z);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glColor3f(0, 1.0, 0.0);
	InterpolatedVertex(0.1, 0.1);
	InterpolatedVertex(0.1, 0.9);
	InterpolatedVertex(0.9, 0.1);
	InterpolatedVertex(0.9, 0.9);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glColor3f(0, 1.0, 1.0);
	InterpolatedVertex(0.3, 0.9);
	InterpolatedVertex(0.5, 1.1);
	InterpolatedVertex(0.7, 0.9);
	glEnd();
}

void TestTile::InterpolatedVertex(float x, float y) {
	Vector3f left = data_[0]*y + data_[3]*(1.0-y);
	Vector3f right = data_[1]*y + data_[2]*(1.0-y);

	Vector3f mid = right*x + left*(1-x);

	glVertex3f(mid.x, mid.y, mid.z);
}
