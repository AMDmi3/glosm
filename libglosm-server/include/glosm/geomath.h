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

#ifndef GEOMATH_H
#define GEOMATH_H

#include <math.h>

/* Earth equatorial radius in WGS84 */
#define WGS84_EARTH_EQ_RADIUS 6378137.0

/* Earth equatorial length */
#define WGS84_EARTH_EQ_LENGTH (M_PI * 2.0 * WGS84_EARTH_EQ_RADIUS)

/* Earth polar radius in WGS84 */
#define WGS84_EARTH_POLE_RADIUS 6356752.314245

/* Zero meridian offset (rel to Greenwich) in WGS84 */
#define WGS84_LONGITUDE_OFFSET 5.31/3600.0

/*
 * These were chosen out of 7 mercator and 4 unmercator
 * functions as fastest on common i386/amd64 hardware; these also
 * happen to give moderate precision. For specific purpose (e.g.
 * maximal performance on specific hardware or top precision, other
 * functions may be chosen.
 */

/** Mercator projection */
inline double mercator(double x) {
	return 0.5*log((1.0+sin(x))/(1.0-sin(x)));
}

/** Mercator un-projection */
inline double unmercator(double x) {
	return 2.0*atan(tanh(0.5*x));
}

#endif // GEOMATH_H
