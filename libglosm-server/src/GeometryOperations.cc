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

bool IntersectSegmentWithHorizontal(const Vector3i& one, const Vector3i& two, osmint_t y, Vector3i& out) {
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

bool IntersectSegmentWithVertical(const Vector3i& one, const Vector3i& two, osmint_t x, Vector3i& out) {
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

bool IntersectPlaneWithVertical(const Vector3i& a, const Vector3i& b, const Vector3i& c, const Vector2i& xy, Vector3i& out) {
	/* Equation of a plane:
	 * |  x-x1  y-y1  z-z1 |
	 * | x2-x1 y2-y1 z2-y1 | = 0
	 * | x3-x1 y3-y1 z3-y1 |
	 *
	 * Determinant:
	 * | a11 a12 a13 |
	 * | a21 a22 a23 | = a11a22a33 − a11a23a32 − a12a21a33 + a12a23a31 + a13a21a32 − a13a22a31
	 * | a31 a32 a33 |
	 *
	 * In terms of array:
	 * | 0 1 2 |
	 * | 3 4 5 | = 0*4*8 - 0*5*7 - 1*3*8 + 1*5*6 + 2*3*7 - 2*4*6
	 * | 6 7 8 |
	 */
	float m[9] = {
		(float)(xy.x - a.x), (float)(xy.y - a.y), 0.0 /* to find */,
		(float)(b.x - a.x), (float)(b.y - a.y), (float)(b.z - a.z),
		(float)(c.x - a.x), (float)(c.y - a.y), (float)(c.z - a.z)
	};

	float divisor = m[3]*m[7] - m[4]*m[6];

	if (fabsf(divisor) < std::numeric_limits<float>::epsilon())
		return false;

	float divided = -m[0]*m[4]*m[8] + m[0]*m[5]*m[7] + m[1]*m[3]*m[8] - m[1]*m[5]*m[6];

	out = Vector3i(xy.x, xy.y, (osmint_t)round(divided/divisor) + a.z);
	return true;
}

bool IntersectSegmentWithBBoxSide(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, BBoxi::Side side, Vector3i& out) {
	switch (side) {
	case BBoxi::LEFT:
		return IntersectSegmentWithVertical(one, two, bbox.left, out);
	case BBoxi::BOTTOM:
		return IntersectSegmentWithHorizontal(one, two, bbox.bottom, out);
	case BBoxi::RIGHT:
		return IntersectSegmentWithVertical(one, two, bbox.right, out);
	case BBoxi::TOP:
		return IntersectSegmentWithHorizontal(one, two, bbox.top, out);
	default:
		return false;
	}
}


BBoxi::Side IntersectSegmentWithBBox(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, Vector3i& out) {
	if (IntersectSegmentWithVertical(one, two, bbox.left, out) && bbox.Contains(out))
		return BBoxi::LEFT;

	if (IntersectSegmentWithHorizontal(one, two, bbox.bottom, out)  && bbox.Contains(out))
		return BBoxi::BOTTOM;

	if (IntersectSegmentWithVertical(one, two, bbox.right, out) && bbox.Contains(out))
		return BBoxi::RIGHT;

	if (IntersectSegmentWithHorizontal(one, two, bbox.top, out) && bbox.Contains(out))
		return BBoxi::TOP;

	return BBoxi::NONE;
}

BBoxi::Side IntersectSegmentWithBBox2(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, Vector3i& out) {
	if (IntersectSegmentWithHorizontal(one, two, bbox.top, out) && bbox.Contains(out))
		return BBoxi::TOP;

	if (IntersectSegmentWithVertical(one, two, bbox.right, out) && bbox.Contains(out))
		return BBoxi::RIGHT;

	if (IntersectSegmentWithHorizontal(one, two, bbox.bottom, out)  && bbox.Contains(out))
		return BBoxi::BOTTOM;

	if (IntersectSegmentWithVertical(one, two, bbox.left, out) && bbox.Contains(out))
		return BBoxi::LEFT;

	return BBoxi::NONE;
}

bool CropSegmentByBBox(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, Vector3i& outone, Vector3i& outtwo) {
	if (bbox.Contains(one)) {
		/* one is in the bbox */
		outone = one;
		if (bbox.Contains(two)) {
			/* both points are in the bbox */
			outtwo = two;
		} else {
			/* other point is outside - find intersection */
			return IntersectSegmentWithBBox(one, two, bbox, outtwo);
		}
	} else if (bbox.Contains(two)) {
		/* two is inside, one is outside */
		outtwo = two;
		return IntersectSegmentWithBBox(one, two, bbox, outone);
	}

	/* both points are outside, find two points of intersection */
	return IntersectSegmentWithBBox(one, two, bbox, outone) && IntersectSegmentWithBBox2(one, two, bbox, outtwo);
}

Vector3d ToLocalMetric(Vector3i what, Vector3i ref) {
	const double coslat = cos(ref.y * GEOM_DEG_TO_RAD);

	double dx = (double)(what.x - ref.x) / GEOM_LONSPAN * WGS84_EARTH_EQ_LENGTH * coslat;
	double dy = (double)(what.y - ref.y) / GEOM_LONSPAN * WGS84_EARTH_EQ_LENGTH;
	double dz = (double)(what.z - ref.z) / GEOM_UNITSINMETER;

	return Vector3f(dx, dy, dz);
}

Vector3i FromLocalMetric(Vector3d what, Vector3i ref) {
	const double coslat = cos(ref.y * GEOM_DEG_TO_RAD);

	int x = ref.x;
	if (coslat > std::numeric_limits<double>::epsilon())
		x += round(what.x * GEOM_LONSPAN / WGS84_EARTH_EQ_LENGTH / coslat);

	int y = ref.y + round(what.y * GEOM_LONSPAN / WGS84_EARTH_EQ_LENGTH);
	int z = ref.z + round(what.z * GEOM_UNITSINMETER);

	return Vector3i(x, y, z);
}

