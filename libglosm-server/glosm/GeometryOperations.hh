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

#ifndef GEOMETRYOPERATIONS_HH
#define GEOMETRYOPERATIONS_HH

#include <glosm/Math.hh>
#include <glosm/BBox.hh>

/**
 * Intersect segment with horizontal line
 *
 * @param one first point of segment
 * @param two second point of segment
 * @param y y-coordinate for line
 * @param out reference to output value
 * @return whether there was an intersection
 */
bool IntersectSegmentWithHorizontal(const Vector3i& one, const Vector3i& two, osmint_t y, Vector3i& out);

/**
 * Intersect segment with vertical line
 *
 * @param one first point of segment
 * @param two second point of segment
 * @param x x-coordinate of line
 * @param out reference to output value
 * @return whether there was an intersection
 */
bool IntersectSegmentWithVertical(const Vector3i& one, const Vector3i& two, osmint_t x, Vector3i& out);

/**
 * Intersect segment with one of bounding box sides
 *
 * @param one first point of segment
 * @param two second point of segment
 * @param bbox bounding box
 * @param side side of bounding box
 * @param out reference to output value
 * @return whether there was an intersection
 */
bool IntersectSegmentWithBBoxSide(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, BBoxi::Side side, Vector3i& out);

/**
 * Intersect segment with one of bounding box sides (non-inclusive version)
 *
 * @see IntersectSegmentWithBBoxSide
 */
static inline bool IntersectSegmentWithBBoxSideNI(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, BBoxi::Side side, Vector3i& out) {
	return IntersectSegmentWithBBoxSide(one, two, bbox, side, out) && out != one && out != two;
}

/**
 * Intersect segment with bounding box
 *
 * @param one first point of segment
 * @param two second point of segment
 * @param bbox bounding box
 * @param out reference to output value
 * @return whether there was an intersection
 */
BBoxi::Side IntersectSegmentWithBBox(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, Vector3i& out);

/**
 * Intersect segment with bounding box
 *
 * This function checks intersection with sides of the bounding box
 * in the reverse order compared to IntersectSegmentWithBBox, so if
 * there are two intersection points, IntersectSegmentWithBBox2 will
 * find another one
 *
 * @param one first point of segment
 * @param two second point of segment
 * @param bbox bounding box
 * @param out reference to output value
 * @return whether there was an intersection
 */
BBoxi::Side IntersectSegmentWithBBox2(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, Vector3i& out);

/**
 * Crops segment with bounding box
 *
 * @param one first point of segment
 * @param two second point of segment
 * @param bbox bounding box
 * @param outone (output) first point of cropped segment
 * @param outone (output) second point of cropped segment
 * @return whether there was an intersection
 */
bool CropSegmentByBBox(const Vector3i& one, const Vector3i& two, const BBoxi& bbox, Vector3i& outone, Vector3i& outtwo);

Vector3d ToLocalMetric(const Vector3i& what, const Vector3i& ref);
Vector3i FromLocalMetric(const Vector3d& what, const Vector3i& ref);

float ApproxDistanceSquare(const BBoxi& bbox, const Vector3i& vec);

#endif
