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

	/* using sqrt(1-sin^2) instead of cos and vice versa gives
	 * ~30% performance gain (thx Komzpa). Precision loss this
	 * brings should be insignificant */
	double cosy = cos(point_angle_y);
	double sinx = sin(point_angle_x);
	Vector3d point_vector(
		(WGS84_EARTH_EQ_RADIUS + point_height) * sinx * cosy,
		(WGS84_EARTH_EQ_RADIUS + point_height) * sqrt(1.0 - cosy * cosy),
		(WGS84_EARTH_EQ_RADIUS + point_height) * sqrt(1.0 - sinx * sinx) * cosy
	);

	/* additional ~20% performance gain */
	double cosay = cos(ref_angle_y);
	double sinay = sqrt(1.0 - cosay * cosay);
	return Vector3f(
		point_vector.x,
		point_vector.y * cosay - point_vector.z * sinay,
		point_vector.y * sinay + point_vector.z * cosay - WGS84_EARTH_EQ_RADIUS - ref_height
	);
}

Vector3i SphericalProjection::UnProjectImpl(const Vector3f& point, const Vector3i& ref) {
	throw std::runtime_error("SphericalProjection::UnProjectImpl not implemented");
	return Vector3i();
}
