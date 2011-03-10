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

#include <glosm/Math.hh>
#include <glosm/BBox.hh>

#include <vector>

/**
 * 3D geometry in global fixed-point coordinates.
 *
 * This class represents portable geometry which comes from
 * GeometryGenerator and may be stored, serialized/deserialized,
 * transferred via the net and in the end is to be converted to
 * local floating-point geometry for rendering.
 *
 * @todo this is pretty non-general approach: triangles and quads
 * should likely be merged into a single primitive. Need testing for
 * speed impact on quads vs. indexed triangle strips. If strips don't
 * bring too buch performance drop, quads should be eliminated. This
 * will also open door for further geom optimizations like merging
 * triangle groups into strips and fans.
 *
 * @note just for a note: most of polygon geometry generated for urban
 * area currently are quads (~10x more quads than triangles). Changing
 * quads to triangle pairs is 12% more geometry generation time, 20%
 * less fps and more memory, so for now they're quite useful.
 */
class Geometry {
public:
	typedef std::vector<Vector3i> VertexVector;
	typedef std::vector<int> LengthVector;

protected:
	VertexVector lines_;
	VertexVector triangles_;
	VertexVector quads_;

protected:
	VertexVector convex_vertices_;
	LengthVector convex_lengths_;

public:
	Geometry();

	void AddLine(const Vector3i& a, const Vector3i& b);
	void AddTriangle(const Vector3i& a, const Vector3i& b, const Vector3i& c);
	void AddQuad(const Vector3i& a, const Vector3i& b, const Vector3i& c, const Vector3i& d);
	void AddConvex(const std::vector<Vector3i>& v);

	void StartConvex(const Vector3i& v);
	void AppendConvex(const Vector3i& v);

	const std::vector<Vector3i>& GetLines() const;
	const std::vector<Vector3i>& GetTriangles() const;
	const std::vector<Vector3i>& GetQuads() const;

	const VertexVector& GetConvexVertices() const;
	const LengthVector& GetConvexLengths() const;

	void Append(const Geometry& other);
	void AppendCropped(const Geometry& other, const BBoxi& bbox);

	void AddCroppedConvex(const Vector3i* v, unsigned int size, const BBoxi& bbox);

	void Serialize() const;
	void DeSerialize();
};

#endif
