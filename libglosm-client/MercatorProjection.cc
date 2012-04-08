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

#include <glosm/MercatorProjection.hh>

#include <glosm/geomath.h>

#include <cmath>

MercatorProjection::MercatorProjection() : Projection(&ProjectImpl, &UnProjectImpl) {
}

Vector3f MercatorProjection::ProjectImpl(const Vector3i& point, const Vector3i& ref) {
	return Vector3f(
			(double)((osmlong_t)point.x - ref.x) * GEOM_DEG_TO_RAD,
			mercator((double)point.y * GEOM_DEG_TO_RAD) - mercator((double)ref.y * GEOM_DEG_TO_RAD),
			(double)(point.z - ref.z) / GEOM_UNITSINMETER / (WGS84_EARTH_EQ_RADIUS * cos((double)(point.y * GEOM_DEG_TO_RAD)))
		);
}

Vector3i MercatorProjection::UnProjectImpl(const Vector3f& point, const Vector3i& ref) {
	double y = unmercator((double)point.y + mercator((double)ref.y * GEOM_DEG_TO_RAD));
	return Vector3i(
			ref.x + (osmint_t)round(point.x * GEOM_RAD_TO_DEG),
			(osmint_t)round(y * GEOM_RAD_TO_DEG),
			ref.z + (osmint_t)round((double)point.z * GEOM_UNITSINMETER * WGS84_EARTH_EQ_RADIUS * cos(y))
		);
}
