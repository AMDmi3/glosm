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

#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include "Math.hh"

#include <vector>

/**
 * 3D geometry in global fixed-point coordinates.
 *
 * This class represents portable geometry which comes from
 * GeometryGenerator and may be stored, serialized/deserialized,
 * transferred via the net and in the end is to be converted to
 * local floating-point geometry for rendering.
 */
class Geometry {
protected:
	typedef std::vector<Vector3i> VertexVector;

protected:
	VertexVector lines_;
	VertexVector triangles_;
	VertexVector quads_;

public:
	void AddLine(const Vector3i& a, const Vector3i& b);
	void AddTriangle(const Vector3i& a, const Vector3i& b, const Vector3i& c);
	void AddQuad(const Vector3i& a, const Vector3i& b, const Vector3i& c, const Vector3i& d);

	const std::vector<Vector3i>& GetLines() const;
	const std::vector<Vector3i>& GetTriangles() const;
	const std::vector<Vector3i>& GetQuads() const;

	void Serialize() const;
	void DeSerialize();
};

#endif
