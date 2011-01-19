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

#ifndef MATH_HH
#define MATH_HH

#include <cmath>
#include <limits>

#include <glosm/osmtypes.h>

template <typename T>
struct LongType {
    typedef T type;
};

template<>
struct LongType<osmint_t> {
    typedef osmlong_t type;
};

template <typename T>
struct Vector3;

/**
 * Template 2D vector class
 */
template <typename T>
struct Vector2 {
    typedef typename LongType<T>::type LT;

	/* ctors */
	Vector2(): x(0), y(0) {}
	Vector2(T x_, T y_): x(x_), y(y_) {}
	Vector2(const Vector2<T>& v): x(v.x), y(v.y) {}
	Vector2(const Vector3<T>& v): x(v.x), y(v.y) {}

	/* static ctoroids */
	template <typename TT>
	static Vector2<T> FromYaw(const TT yaw) { return Vector2(std::sin(yaw), std::cos(yaw)); }

	template <typename TT>
	Vector2(const Vector2<TT>& v): x(v.x), y(v.y) {}

    /* */
    Vector2<LT> LongVector() const {
        return Vector2<LT>(*this);
    }

	/* operators */
	Vector2<T> operator- () const { return Vector2<T>(-x, -y); }

	Vector2<T> operator+ (const Vector2<T>& other) const { return Vector2<T>(x + other.x, y + other.y); }
	Vector2<T> operator- (const Vector2<T>& other) const { return Vector2<T>(x - other.x, y - other.y); }
	Vector2<T> operator* (const Vector2<T>& other) const { return Vector2<T>(x * other.x, y * other.y); }
	Vector2<T> operator/ (const Vector2<T>& other) const { return Vector2<T>(x / other.x, y / other.y); }

	Vector2<T> operator* (const T v) const { return Vector2<T>(x * v, y * v); }
	Vector2<T> operator/ (const T v) const { return Vector2<T>(x / v, y / v); }

	/* mutating operators */
	Vector2<T>& operator= (const Vector2<T>& other) { x = other.x; y = other.y; return *this; }

	Vector2<T>& operator+= (const Vector2<T>& other) { x += other.x; y += other.y; return *this; }
	Vector2<T>& operator-= (const Vector2<T>& other) { x -= other.x; y -= other.y; return *this; }
	Vector2<T>& operator*= (const Vector2<T>& other) { x *= other.x; y *= other.y; return *this; }
	Vector2<T>& operator/= (const Vector2<T>& other) { x /= other.x; y /= other.y; return *this; }

	Vector2<T>& operator*= (const T& v) { x *= v; y *= v; return *this; }
	Vector2<T>& operator/= (const T& v) { x /= v; y /= v; return *this; }

	/* comparison */
	bool operator== (const Vector2<T>& other) const { return x == other.x && y == other.y; }

	/* functions */
	T Length() { return std::sqrt((LT)x*(LT)x + (LT)y*(LT)y); }
	LT LengthSquare() { return (LT)x*(LT)x + (LT)y*(LT)y; }

	LT DotProduct(const Vector2<T>& other) const { return (LT)x*(LT)other.x + (LT)y*(LT)other.y; }
	LT CrossProduct(const Vector2<T>& other) const { return (LT)x*(LT)other.y - (LT)y*(LT)other.x; }

	void Normalize() {
		T len = Length();
		if (len == 0)
			return;

		x /= len;
		y /= len;
	}

	Vector2<T> Normalized() {
		T len = Length();
		if (len == 0)
			return Vector2<T>(); /* Zero vector */

		return Vector2<T>(x / len, y / len);
	}

	bool IsInTriangle(const Vector2<T>& v1, const Vector2<T>& v2, const Vector2<T>& v3) {
		LT a = (v2-v1).CrossProduct(*this - v1);
		LT b = (v3-v2).CrossProduct(*this - v2);
		LT c = (v1-v3).CrossProduct(*this - v3);

		return (a < 0 && b < 0 && c < 0) || (a > 0 && b > 0 && c > 0);
	}

	/* data */
	T x;
	T y;
};

/**
 * Template 3D vector class
 */
template <typename T>
struct Vector3 {
    typedef typename LongType<T>::type LT;

	/* ctors */
	Vector3(): x(0), y(0), z(0) {}
	Vector3(T x_, T y_): x(x_), y(y_), z(0) {}
	Vector3(T x_, T y_, T z_): x(x_), y(y_), z(z_) {}

	Vector3(const Vector2<T>& v): x(v.x), y(v.y), z(0) {}
	Vector3(const Vector2<T>& v, T z_): x(v.x), y(v.y), z(z_) {}
	Vector3(const Vector3<T>& v): x(v.x), y(v.y), z(v.z) {}

	template <typename TT>
	Vector3(const Vector3<TT>& v): x(v.x), y(v.y), z(v.z) {}

	/* static ctoroids */
	template <typename TT>
	static Vector3<T> FromYawPitch(const TT yaw, const TT pitch) { return Vector3(sin(yaw)*cos(pitch), cos(yaw)*cos(pitch), sin(pitch)); }

    /* */
    Vector3<LT> LongVector() const {
        return Vector3<LT>(*this);
    }

	/* operators */
	Vector3<T> operator- () const { return Vector3<T>(-x, -y, -z); }

	Vector3<T> operator+ (const Vector3<T>& other) const { return Vector3<T>(x + other.x, y + other.y, z + other.z); }
	Vector3<T> operator- (const Vector3<T>& other) const { return Vector3<T>(x - other.x, y - other.y, z - other.z); }
	Vector3<T> operator* (const Vector3<T>& other) const { return Vector3<T>(x * other.x, y * other.y, z * other.z); }
	Vector3<T> operator/ (const Vector3<T>& other) const { return Vector3<T>(x / other.x, y / other.y, z / other.z); }

	Vector3<T> operator* (const T v) const { return Vector3<T>(x * v, y * v, z * v); }
	Vector3<T> operator/ (const T v) const { return Vector3<T>(x / v, y / v, z / v); }

	/* mutating operators */
	Vector3<T>& operator= (const Vector3<T>& other) { x = other.x; y = other.y; z = other.z; return *this; }

	Vector3<T>& operator+= (const Vector3<T>& other) { x += other.x; y += other.y; z += other.z; return *this; }
	Vector3<T>& operator-= (const Vector3<T>& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
	Vector3<T>& operator*= (const Vector3<T>& other) { x *= other.x; y *= other.y; z *= other.z; return *this; }
	Vector3<T>& operator/= (const Vector3<T>& other) { x /= other.x; y /= other.y; z /= other.z; return *this; }

	Vector3<T>& operator*= (const T& v) { x *= v; y *= v; z *= v; return *this; }
	Vector3<T>& operator/= (const T& v) { x /= v; y /= v; z /= v; return *this; }

	/* comparison */
	bool operator== (const Vector3<T>& other) const { return x == other.x && y == other.y && z == other.z; }

	/* functions */
	T Length() { return std::sqrt((LT)x*(LT)x + (LT)y*(LT)y + (LT)z*(LT)z); }
	LT LengthSquare() { return (LT)x*(LT)x + (LT)y*(LT)y + (LT)z*(LT)z; }

	LT DotProduct(const Vector3<T>& other) const { return (LT)x*(LT)other.x + (LT)y*(LT)other.y + (LT)z*(LT)other.z; }
	Vector3<LT> CrossProduct(const Vector3<T>& other) const { return Vector3<LT>((LT)y*(LT)other.z - (LT)z*(LT)other.y, (LT)z*(LT)other.x - (LT)x*(LT)other.z, (LT)x*(LT)other.y - (LT)y*(LT)other.x); }

	void Normalize() {
		T len = Length();
		if (len == 0)
			return;

		x /= len;
		y /= len;
		z /= len;
	}

	Vector3<T> Normalized() {
		T len = Length();
		if (len == 0)
			return Vector3<T>(); /* Zero vector */

		return Vector3<T>(x / len, y / len, z / len);
	}

	/* data */
	T x;
	T y;
	T z;
};

typedef Vector2<osmint_t> Vector2i;
typedef Vector2<osmlong_t> Vector2l;
typedef Vector2<float> Vector2f;
typedef Vector2<double> Vector2d;

typedef Vector3<osmint_t> Vector3i;
typedef Vector3<osmlong_t> Vector3l;
typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;

#endif
