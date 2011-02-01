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

#include <glosm/SphericalProjection.hh>

#include <glosm/geomath.h>

#include <cmath>
#include <stdexcept>

SphericalProjection::SphericalProjection() : Projection(&ProjectImpl, &UnProjectImpl) {
}

Vector3f SphericalProjection::ProjectImpl(const Vector3i& point, const Vector3i& ref) {
	double point_angle_x = ((double)point.x - (double)ref.x) * GEOM_DEG_TO_RAD;
	double point_angle_y = (double)point.y * GEOM_DEG_TO_RAD;
	double point_height = (double)point.z / GEOM_UNITSINMETER;

	double ref_angle_y = (double)ref.y * GEOM_DEG_TO_RAD;
	double ref_height = (double)ref.z / GEOM_UNITSINMETER;

	/* XXX: this can benefit from sincos() on Linux */
	double cosx = cos(point_angle_x);
	double cosy = cos(point_angle_y);
	double sinx = sin(point_angle_x);
	double siny = sin(point_angle_y);
	Vector3d point_vector(
		(WGS84_EARTH_EQ_RADIUS + point_height) * sinx * cosy,
		(WGS84_EARTH_EQ_RADIUS + point_height) * siny,
		(WGS84_EARTH_EQ_RADIUS + point_height) * cosx * cosy
	);

	/* XXX: this can benefit from sincos() on Linux */
	double cosay = cos(ref_angle_y);
	double sinay = sin(ref_angle_y);
	return Vector3f(
		point_vector.x,
		point_vector.y * cosay - point_vector.z * sinay,
		point_vector.y * sinay + point_vector.z * cosay - WGS84_EARTH_EQ_RADIUS - ref_height
	);
}

Vector3i SphericalProjection::UnProjectImpl(const Vector3f& point, const Vector3i& ref) {
	double ref_angle_y = (double)ref.y * GEOM_DEG_TO_RAD;
	double ref_height = (double)ref.z / GEOM_UNITSINMETER;

	/* XXX: this can benefit from sincos() on Linux */
	double cosay = cos(ref_angle_y);
	double sinay = sin(ref_angle_y);
	Vector3d point_vector(
			point.x,
			sinay * (WGS84_EARTH_EQ_RADIUS + ref_height + point.z) + cosay * point.y,
			cosay * (WGS84_EARTH_EQ_RADIUS + ref_height + point.z) - sinay * point.y
		);

	Vector3d spherical(
		atan2(point_vector.x, point_vector.z),
		atan2(point_vector.y, sqrt(point_vector.x * point_vector.x + point_vector.z * point_vector.z)),
		point_vector.Length() - WGS84_EARTH_EQ_RADIUS
	);

	return Vector3i(
			ref.x + (osmint_t)round(spherical.x * GEOM_RAD_TO_DEG),
			(osmint_t)round(spherical.y * GEOM_RAD_TO_DEG),
			(osmint_t)round(spherical.z * GEOM_UNITSINMETER)
		);
}
