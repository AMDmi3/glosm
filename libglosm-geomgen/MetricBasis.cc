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

#include <glosm/MetricBasis.hh>
#include <glosm/GeometryOperations.hh>

MetricBasis::MetricBasis(const Vector3i ref, const Vector3d& x, const Vector3d& y, const Vector3d& z):
	ref_(ref),
	x_(x),
	y_(y),
	z_(z) {
}

MetricBasis::MetricBasis(const Vector3i ref, const Vector3d& x):
	ref_(ref),
	x_(x),
	y_(-x.CrossProduct(Vector3d(0.0, 0.0, 1.0))),
	z_(Vector3d(0.0, 0.0, 1.0)) {
}

Vector3i MetricBasis::Get(double x, double y, double z) {
	return FromLocalMetric(x_ * x + y_ * y + z_ * z, ref_);
}
