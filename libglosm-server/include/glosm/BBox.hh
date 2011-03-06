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

#ifndef BBOX_HH
#define BBOX_HH

#include <glosm/Math.hh>

/**
 * Template axis-aligned bounding box class
 */
template <typename T>
struct BBox {
public:
	/**
	 * Sides of boundng box
	 */
	enum Side {
		NONE = 0,
		LEFT = 1,
		BOTTOM = 2,
		RIGHT = 3,
		TOP = 4
	};

public:
	typedef typename LongType<T>::type LT;

public:
	/* CTORS */

	/**
	 * Constructs empty bounding box
	 */
	BBox(): left(std::numeric_limits<T>::max()), bottom(std::numeric_limits<T>::max()), right(std::numeric_limits<T>::min()), top(std::numeric_limits<T>::min()) {
	}

	/**
	 * Constructs bbox by two corners
	 *
	 * @param one one corner
	 * @param two another corner
	 */
	BBox(const Vector2<T>& one, const Vector2<T>& two) {
		if (one.x < two.x) {
			left = one.x;
			right = two.x;
		} else {
			left = two.x;
			right = one.x;
		}

		if (one.y < two.y) {
			bottom = one.y;
			top = two.y;
		} else {
			bottom = two.y;
			top = one.y;
		}
	}

	/**
	 * Constructs bbox by its sides
	 *
	 * @param l left side
	 * @param b bottom side
	 * @param r right side
	 * @param t top side
	 */
	BBox(T l, T b, T r, T t): left(l), bottom(b), right(r), top(t) {
	}

	/**
	 * Constructs copy of other
	 */
	BBox(const BBox<T>& other): left(other.left), bottom(other.bottom), right(other.right), top(other.top) {
	}

	/* STATIC CTORS */

	/**
	 * Creates bbox that contains all possible points
	 */
	static BBox<T> Full() {
		return BBox<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
	}

	/**
	 * Creates bbox that contains no points
	 */
	static BBox<T> Empty() {
		return BBox<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), std::numeric_limits<T>::min());
	}

	/* following are specialized only for <osmint_t>, see BBox.cc */

	/**
	 * Creates bbox that covers earth surface
	 *
	 * @note only specialized for <osmint_t>, see BBox.cc
	 */
	static BBox<T> ForEarth();

	/**
	 * Creates bbox for specified mercator tile
	 *
	 * @param zoom zoom
	 * @param x x-coordinate (longitued)
	 * @param y t-coordinate (latitude)
	 *
	 * Mercator tile numbering is similar with those used in mapnik
	 *
	 * @note only specialized for <osmint_t>, see BBox.cc
	 */
	static BBox<T> ForMercatorTile(int zoom, int x, int y);

	/**
	 * Creates bbox for specified geo tile
	 *
	 * @param zoom zoom
	 * @param x x-coordinate (longitued)
	 * @param y t-coordinate (latitude)
	 *
	 * Geo tiles uniformly split earth rectangle and are numbered from topleft
	 *
	 * @note only specialized for <osmint_t>, see BBox.cc
	 */
	static BBox<T> ForGeoTile(int zoom, int x, int y);

	/* OPERATORS */

	/**
	 * Returns bbox shifted in direction specified by vector
	 */
	BBox<T> operator+ (const Vector2<T>& shift) const {
		return BBox<T>(left + shift.x, bottom + shift.y, right + shift.x, top + shift.y);
	}

	/**
	 * Returns bbox shifted in direction opposide to one specified by vector
	 */
	BBox<T> operator- (const Vector2<T>& shift) const {
		return BBox<T>(left - shift.x, bottom - shift.y, right - shift.x, top - shift.y);
	}

	/**
	 * Shifts bbox in direction specified by vector
	 */
	BBox<T> operator+= (const Vector2<T>& shift) {
		left += shift.x;
		right += shift.x;
		bottom += shift.y;
		top += shift.y;
		return *this;
	}

	/**
	 * Shifts bbox in direction opposide to one specified by vector
	 */
	BBox<T> operator-= (const Vector2<T>& shift) {
		left -= shift.x;
		right -= shift.x;
		bottom -= shift.y;
		top -= shift.y;
		return *this;
	}

	/* MUTATORS */

	/**
	 * Includes a single point into bbox, expanding it if necessary
	 */
	void Include(const Vector2<T>& point) {
		if (point.x < left)
			left = point.x;
		if (point.x > right)
			right = point.x;
		if (point.y < bottom)
			bottom = point.y;
		if (point.y > top)
			top = point.y;
	}

	/**
	 * Includes another bbox into bbox, expanding it if necessary
	 */
	void Include(const BBox<T>& bbox) {
		if (bbox.left < left)
			left = bbox.left;
		if (bbox.right > right)
			right = bbox.right;
		if (bbox.bottom < bottom)
			bottom = bbox.bottom;
		if (bbox.top > top)
			top = bbox.top;
	}

	/**
	 * Returns geometrical center of a bbox
	 */
	inline Vector2<T> GetCenter() const {
		return Vector2<T>(((LT)left + (LT)right)/2, ((LT)top + (LT)bottom)/2);
	}

	/**
	 * Returns bottom left corner of a bbox
	 */
	inline Vector2<T> GetBottomLeft() const {
		return Vector2<T>(left, bottom);
	}

	/**
	 * Returns bottom right corner of a bbox
	 */
	inline Vector2<T> GetBottomRight() const {
		return Vector2<T>(right, bottom);
	}

	/**
	 * Returns top left corner of a bbox
	 */
	inline Vector2<T> GetTopLeft() const {
		return Vector2<T>(left, top);
	}

	/**
	 * Returns top right corner of a bbox
	 */
	inline Vector2<T> GetTopRight() const {
		return Vector2<T>(right, top);
	}

	/* CHECKS */

	/**
	 * Checks whether bbox is empty
	 *
	 * @return true if bbox is empty, false otherwise
	 */
	inline bool IsEmpty() const {
		return left > right || bottom > top;
	}

	/**
	 * Checks whether bbox contains specific point
	 *
	 * @return true if bbox contains point, false otherwise
	 */
	inline bool Contains(const Vector2<T>& point) const {
		return point.x >= left && point.x <= right && point.y >= bottom && point.y <= top;
	}

	/**
	 * Checks whether bbox intersects with another bbox
	 *
	 * @return true if bbox contains another bbox, false otherwise
	 */
	inline bool Intersects(const BBox<T>& other) const {
		return !(other.right < left || other.left > right || other.top < bottom || other.bottom > top);
	}

	/**
	 * Checks whether point is located to the specific side from bbox
	 */
	inline bool IsPointOutAtSide(const Vector2i& point, Side side) const {
		switch (side) {
		case LEFT: return point.x < left;
		case RIGHT: return point.x > right;
		case TOP: return point.y > top;
		case BOTTOM: return point.y < bottom;
		default: return false;
		}
	}

	/**
	 * Returns point of bbox nearest to specific point
	 */
	template<class V>
	inline Vector2<T> NearestPoint(const V& vec) const {
		if (vec.x < left) {
			/* to the left */
			if (vec.y < bottom)
				return GetBottomLeft();
			else if (vec.y > top)
				return GetTopLeft();
			else
				return Vector2<T>(left, vec.y);
		} else if (vec.x > right) {
			/* to the right */
			if (vec.y < bottom)
				return GetBottomRight();
			else if (vec.y > top)
				return GetTopRight();
			else
				return Vector2<T>(right, vec.y);
		} else {
			if (vec.y < bottom)
				return Vector2<T>(vec.x, bottom);
			else if (vec.y > top)
				return Vector2<T>(vec.x, top);
			else
				return vec; /* inside bbox */
		}
	}

	/**
	 * Returns point of bbox farthest from specific point
	 */
	template<class V>
	inline Vector2<T> FarthestPoint(const V& vec) const {
		Vector2<T> center = GetCenter();
		if (vec.x < center.x) {
			if (vec.y < center.y)
				return GetTopRight();
			else
				return GetBottomRight();
		} else {
			if (vec.y < center.y)
				return GetTopLeft();
			else
				return GetBottomLeft();
		}
	}

	/* data */
	T left, bottom, right, top;
};

typedef BBox<osmint_t> BBoxi;
typedef BBox<osmlong_t> BBoxl;
typedef BBox<float> BBoxf;
typedef BBox<double> BBoxd;

#endif
