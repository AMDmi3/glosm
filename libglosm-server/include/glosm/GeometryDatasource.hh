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

#ifndef GEOMETRYDATASOURCE_HH
#define GEOMETRYDATASOURCE_HH

#include <glosm/Math.hh>
#include <glosm/BBox.hh>

class Geometry;

/**
 * Abstract base class for all Geometry sources including Geometry
 * generators.
 */
class GeometryDatasource {
public:
	enum Flags {
		LOWRES = 0x01,
	};

public:
	virtual void GetGeometry(Geometry& geometry, const BBoxi& bbox, int flags = 0) const = 0;

	/** Returns the center of available area */
	virtual Vector2i GetCenter() const {
		return Vector2i(0, 0);
	}

	/** Returns the bounding box of available area */
	virtual BBoxi GetBBox() const {
		return BBoxi::ForEarth();
	}
};

#endif
