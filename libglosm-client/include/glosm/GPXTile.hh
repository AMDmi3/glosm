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

#ifndef GPXTILE_HH
#define GPXTILE_HH

#include <glosm/Tile.hh>
#include <glosm/NonCopyable.hh>
#include <glosm/BBox.hh>

#include <memory>
#include <vector>

template <class T>
class VertexBuffer;

class Projection;

/**
 * A tile of GPX points
 *
 * This tile type is used by GPXLayer
 */
class GPXTile : public Tile, private NonCopyable {
protected:
	std::auto_ptr<VertexBuffer<Vector3f> > points_;

	size_t size_;

public:
	/**
	 * Constructs tile from vector of points
	 *
	 * @param projection projection used to convert fixed-point geometry
	 * @param points source points
	 * @param ref reference point of this tile
	 * @param bbox bounding box of this tile
	 */
	GPXTile(const Projection& projection, const std::vector<Vector3i>& points, const Vector2i& ref, const BBoxi& bbox);

	/**
	 * Destructor
	 */
	virtual ~GPXTile();

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
