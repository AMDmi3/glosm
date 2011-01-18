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

#ifndef GEOMETRYTILE_HH
#define GEOMETRYTILE_HH

#include <glosm/Tile.hh>

#include <glosm/NonCopyable.hh>

#include <memory>

class SimpleVertexBuffer;

class Projection;
class GeometryDatasource;

/**
 * A tile of renderable geometry
 *
 * This tile type is used in GeometryLayer
 */
class GeometryTile : public Tile, NonCopyable {
protected:
	std::auto_ptr<SimpleVertexBuffer> lines_;
	std::auto_ptr<SimpleVertexBuffer> triangles_;
	std::auto_ptr<SimpleVertexBuffer> quads_;

public:
	GeometryTile(const Projection& p, const GeometryDatasource& ds, const Vector2i& ref, const BBoxi& bbox);
	virtual ~GeometryTile();

	virtual void Render() const;
};

#endif
