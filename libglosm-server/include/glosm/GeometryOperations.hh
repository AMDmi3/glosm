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

#ifndef GEOMETRYOPERATIONS_HH
#define GEOMETRYOPERATIONS_HH

#include <glosm/Math.hh>

bool IntersectLineWithHorizontal(const Vector3i& one, const Vector3i& two, osmint_t y, Vector3i& out);
bool IntersectLineWithVertical(const Vector3i& one, const Vector3i& two, osmint_t x, Vector3i& out);

Vector3d ToLocalMetric(Vector3i what, Vector3i ref);
Vector3i FromLocalMetric(Vector3d what, Vector3i ref);

#endif
