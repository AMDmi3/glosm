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

#ifndef GEOMETRYTILE_HH
#define GEOMETRYTILE_HH

#include <glosm/Tile.hh>
#include <glosm/BBox.hh>
#include <glosm/NonCopyable.hh>

#include <glosm/util/gl.h>

#include <memory>
#include <vector>

template<class T>
class VertexBuffer;

class Projection;
class Geometry;

/**
 * A tile of renderable geometry
 *
 * This tile type is used in GeometryLayer
 */
class GeometryTile : public Tile, private NonCopyable {
protected:
	struct Vertex {
		Vector3f pos;
		Vector3f norm;

		Vertex(const Vector3f& p): pos(p) {
		}
	};

protected:
	std::auto_ptr<VertexBuffer<Vector3f> > lines_vertices_;
	std::auto_ptr<VertexBuffer<GLuint> > lines_indices_;

	std::auto_ptr<VertexBuffer<Vertex> > convex_vertices_;
	std::auto_ptr<VertexBuffer<GLuint> > convex_indices_;

	size_t size_;

#ifdef TILE_DEBUG
	Vector3f bound_1[4];
	Vector3f bound_2[40];
#endif

protected:
	void CalcFanNormal(Vertex* vertices, int count);

public:
	/**
	 * Constructs tile from given geometry
	 *
	 * @param projection projection used to convert fixed-point geometry
	 * @param geometry source geometry
	 * @param ref reference point of this tile
	 * @param bbox bounding box of this tile
	 */
	GeometryTile(const Projection& projection, const Geometry& geometry, const Vector2i& ref, const BBoxi& bbox);

	/**
	 * Destructor
	 */
	virtual ~GeometryTile();

	/**
	 * Render this tile
	 */
	virtual void Render();

	/**
	 * Returns tile size in bytes
	 */
	virtual size_t GetSize() const;
};

#endif
