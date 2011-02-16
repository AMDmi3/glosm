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

#ifndef TILE_HH
#define TILE_HH

#include <glosm/Math.hh>

/**
 * Abstract class for all geodata tiles.
 *
 * Tile is just an abstract hunk of geodata ready for rendering.
 * Each tile has center stored in global fixed-point coords, while
 * all the actual data is stored in fixed-point format relative to
 * that center. As the tiles are expected to be small enough, this
 * prevents floating point errors to be noticeable.
 */
class Tile {
protected:
	/**
	 * Global coordinates of tile center.
	 */
	const Vector2i reference_;

public:
	Tile(const Vector2i& ref) : reference_(ref) {}
	virtual ~Tile() {}

	virtual void Render() = 0;

	const Vector2i& GetReference() const {
		return reference_;
	}
};

#endif
