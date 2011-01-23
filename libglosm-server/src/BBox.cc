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

#include <glosm/BBox.hh>
#include <glosm/geomath.h>
#include <math.h>

template<>
BBox<osmint_t> BBox<osmint_t>::ForEarth() {
	return BBox<osmint_t>(-1800000000, -900000000, 1800000000, 900000000);
}

template<>
BBox<osmint_t> BBox<osmint_t>::ForMercatorTile(int zoom, int x, int y) {
	BBox<osmint_t> bbox(
			(osmint_t)round(((double)x) / pow(2.0, zoom) * 3600000000.0 - 1800000000.0),
			(osmint_t)round(-unmercator(((float)y) / powf(2.0, zoom) * M_PI * 2.0 - M_PI) / M_PI * 1800000000.0),
			(osmint_t)round(((double)x + 1.0) / pow(2.0, zoom) * 3600000000.0 - 1800000000.0),
			(osmint_t)round(-unmercator(((float)y + 1.0) / powf(2.0, zoom) * M_PI * 2.0 - M_PI) / M_PI * 1800000000.0)
		);


	return bbox;
}
