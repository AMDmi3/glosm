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

#include <glosm/MercatorProjection.hh>

#include <glosm/geomath.h>

#include <cmath>

MercatorProjection::MercatorProjection() : Projection(&ProjectImpl, &UnProjectImpl) {
}

Vector3f MercatorProjection::ProjectImpl(const Vector3i& point, const Vector3i& ref) {
	return Vector3f(
			(double)((osmlong_t)point.x - ref.x) / 1800000000.0 * M_PI,
			mercator((double)point.y / 1800000000.0 * M_PI) - mercator((double)ref.y / 1800000000.0 * M_PI),
			(double)(point.z - ref.z) / 1000.0 / (WGS84_EARTH_EQ_RADIUS * cos((double)(point.y / 1800000000.0 * M_PI)))
		);
}

Vector3i MercatorProjection::UnProjectImpl(const Vector3f& point, const Vector3i& ref) {
	double y = unmercator((double)point.y + mercator((double)ref.y / 1800000000.0 * M_PI));
	return Vector3i(
			ref.x + (osmint_t)round(point.x / M_PI * 1800000000.0),
			(osmint_t)round(y / M_PI * 1800000000.0),
			ref.z + (osmint_t)round((double)point.z * 1000.0 * WGS84_EARTH_EQ_RADIUS * cos(y))
		);
}
