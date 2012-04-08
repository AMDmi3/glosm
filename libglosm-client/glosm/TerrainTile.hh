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

#ifndef TERRAINTILE_HH
#define TERRAINTILE_HH

#include <glosm/Tile.hh>
#include <glosm/NonCopyable.hh>
#include <glosm/BBox.hh>

#include <glosm/util/gl.h>

#include <memory>
#include <vector>

class SimpleVertexBuffer;
class HeightmapDatasource;

class Projection;

template<class T>
class VertexBuffer;

/**
 * A terrain tile
 *
 * This tile type is used by TerrainLayer
 */
class TerrainTile : public Tile, private NonCopyable {
protected:
	struct TerrainVertex {
		Vector3f pos;
		Vector3f norm;
	};

protected:
	std::auto_ptr<VertexBuffer<TerrainVertex> > vbo_;
	std::auto_ptr<VertexBuffer<GLushort> > ibo_;

	size_t size_;

public:
	TerrainTile(const Projection& projection, HeightmapDatasource& datasource, const Vector2i& ref, const BBoxi& bbox);

	/**
	 * Destructor
	 */
	virtual ~TerrainTile();

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
