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

#ifndef HEIGHTMAPDATASOURCE_HH
#define HEIGHTMAPDATASOURCE_HH

#include <glosm/Math.hh>
#include <glosm/BBox.hh>

#include <vector>

/**
 * Abstract base class for sources of heightmap data.
 */
class HeightmapDatasource {
public:
	struct Heightmap {
		typedef std::vector<osmint_t> HeightmapPoints;

		HeightmapPoints points;
		int             width;
		int             height;

		BBoxi           bbox;
	};

public:
	virtual ~HeightmapDatasource() {}

	virtual void GetHeightmap(const BBoxi& bbox, int extramargin, Heightmap& out) const = 0;
	virtual osmint_t GetHeight(const Vector2i& where) const = 0;
};

#endif
