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

#include <glosm/GeometryOperations.hh>
#include <glosm/geomath.h>
#include <math.h>

bool IntersectLineWithHorizontal(const Vector3i& one, const Vector3i& two, osmint_t y, Vector3i& out) {
	if (one.y < y && two.y < y)
		return false;

	if (one.y > y && two.y > y)
		return false;

	float t = (float)(one.y - y) / (float)(one.y - two.y);

	float x = (float)(two.x - one.x) * t;
	float z = (float)(two.z - one.z) * t;

	out = Vector3i(one.x + round(x), y, one.z + round(z));
	return true;
}

bool IntersectLineWithVertical(const Vector3i& one, const Vector3i& two, osmint_t x, Vector3i& out) {
	if (one.x < x && two.x < x)
		return false;

	if (one.x > x && two.x > x)
		return false;

	float t = (float)(one.x - x) / (float)(one.x - two.x);

	float y = (float)(two.y - one.y) * t;
	float z = (float)(two.z - one.z) * t;

	out = Vector3i(x, one.y + round(y), one.z + round(z));
	return true;
}

Vector3d ToLocalMetric(Vector3i what, Vector3i ref) {
	double lat = ref.y/1800000000.0*M_PI;

	double dx = (double)(what.x - ref.x)/3600000000.0*WGS84_EARTH_EQ_LENGTH*cos(lat);
	double dy = (double)(what.y - ref.y)/3600000000.0*WGS84_EARTH_EQ_LENGTH;
	double dz = (double)(what.z - ref.z)/1000.0;

	return Vector3f(dx, dy, dz);
}

Vector3i FromLocalMetric(Vector3d what, Vector3i ref) {
	double lat = ref.y/1800000000.0*M_PI;

	int x = ref.x + round(what.x*3600000000.0/WGS84_EARTH_EQ_LENGTH/cos(lat));
	int y = ref.y + round(what.y*3600000000.0/WGS84_EARTH_EQ_LENGTH);
	int z = ref.z + round(what.z*1000.0);

	return Vector3i(x, y, z);
}

