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
	typedef typename LongType<T>::type LT;

	/* ctors */
	BBox(Vector2<T> one, Vector2<T> two) {
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

	BBox(T l, T b, T r, T t): left(l), bottom(b), right(r), top(t) {
	}

	BBox(const BBox<T>& b): left(b.left), bottom(b.bottom), right(b.right), top(b.top) {
	}

	/* static `ctors' */
	static BBox<T> Full() {
		return BBox<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
	}

	static BBox<T> Empty() {
		return BBox<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), std::numeric_limits<T>::min());
	}

	/* operators - may be added later if needed: addition/substraction/intersection */

	/* misc */
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

	bool IsEmpty() const {
		return left > right || bottom > top;
	}

	Vector2<T> GetCenter() const {
		return Vector2<T>(((LT)left + (LT)right)/2, ((LT)top + (LT)bottom)/2);
	}

	Vector2<T> GetBottomLeft() const {
		return Vector2<T>(left, bottom);
	}

	Vector2<T> GetTopRight() const {
		return Vector2<T>(right, top);
	}

	bool Contains(const Vector2<T>& v) const {
		return v.x >= left && v.x <= right && v.y >= bottom && v.y <= top;
	}

	/* data */
	T left, bottom, right, top;
};

typedef BBox<osmint_t> BBoxi;
typedef BBox<osmlong_t> BBoxl;
typedef BBox<float> BBoxf;
typedef BBox<double> BBoxd;

#endif
