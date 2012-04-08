/*
 * Copyright (C) 2010-2011 Dmitry Marakasov
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

#ifndef METRICBASIS_HH
#define METRICBASIS_HH

#include <glosm/GeometryOperations.hh>

/**
 * Convenient metric basis class for geometry construction
 */
class MetricBasis {
public:
	/**
	 * Creates basis out of 3 axis vectors (expected to be normalized)
	 */
	MetricBasis(const Vector3i ref, const Vector3d& x, const Vector3d& y, const Vector3d& z);

	/**
	 * Creates basis out of a single axis vectors (expected to be normalized)
	 *
	 * Z axis is considered to be [0, 0, 1] and Y is calculated as
	 * perpendicular to XZ plane
	 */
	MetricBasis(const Vector3i ref, const Vector3d& x);

	/**
	 * Returns basis-relative point as global fixed-point coordinates
	 */
	Vector3i Get(double x, double y, double z);

protected:
	Vector3i ref_;
	Vector3d x_, y_, z_;
};

#endif
