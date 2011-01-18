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

#ifndef PROJECTION_HH
#define PROJECTION_HH

#include <glosm/Math.hh>

#include <vector>

/**
 * Abstract base class for all projections.
 *
 * Provides a way to translate global fixed-point geometry coordinates
 * into relative floating point ones and vice versa.
 *
 * Since OpenGL works with float data, and float has not enough
 * precision to represent geographical coordinates for OpenStreetMap
 * objects (up to 1 meter error on high longitudes), we have to use
 * fixed point format for data interchange and relative floating-point
 * format for rendering.
 *
 * @note: this class is designed in a way that its derivatives can
 *        be safely cast to Projection copied and passed by value.
 */
class Projection {
private:
	typedef Vector3f(*ProjectFunction)(const Vector3i&, const Vector3i&);
	typedef Vector3i(*UnProjectFunction)(const Vector3f&, const Vector3i&);

private:
	ProjectFunction project_;
	UnProjectFunction unproject_;

protected:
	Projection(ProjectFunction pf, UnProjectFunction uf);

public:
	/**
	 * Translates a point from global fixed-point to relative
	 * floating-point coordinate system.
	 *
	 * @param point point to translate
	 * @param ref reference point which denotes the center of
	 *            local coordinate system
	 * @return translated point
	 */
	Vector3f Project(const Vector3i& point, const Vector3i& ref) const;

	/**
	 * Translates a point from relative floating-point to global
	 * fixed-point coordinate system.
	 *
	 * @param point point to translate
	 * @param ref reference point which denotes the center of
	 *            local coordinate system
	 * @return translated point
	 */
	Vector3i UnProject(const Vector3f& point, const Vector3i& ref) const;

	/**
	 * Translates bunch of points from relative floating-point
	 * to global fixed-point coordinate system.
	 *
	 * @param in reference to input vector of points
	 * @param out reference to output vector of translated
	 *            points (which is appended)
	 * @param ref reference point which denotes the center of
	 *            local coordinate system
	 */
	void ProjectPoints(const std::vector<Vector3i>& in, const Vector3i& ref, std::vector<Vector3f>& out) const;
};

#endif
