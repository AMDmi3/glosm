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

/*
 * This is a microbenchmark for projections.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <glosm/geomath.h>

#include <glosm/MercatorProjection.hh>
#include <glosm/SphericalProjection.hh>

void ProjBench(Projection projection) {
	int count = 0;

	struct timeval start, end;
	gettimeofday(&start, NULL);
	for (int lon = GEOM_MINLON; lon <= GEOM_MAXLON; lon += GEOM_UNITSINDEGREE/4) {
		for (int lat = GEOM_MINLAT; lat <= GEOM_MAXLAT; lat += GEOM_UNITSINDEGREE/4) {
			projection.Project(Vector3i(lon, lat, 10), Vector3i(GEOM_UNITSINDEGREE * 45, GEOM_UNITSINDEGREE * 60, 10));
			projection.Project(Vector3i(lon, lat, 10), Vector3i(GEOM_UNITSINDEGREE * -45, GEOM_UNITSINDEGREE * -60, 10));
			projection.Project(Vector3i(lon, lat, 10), Vector3i(GEOM_UNITSINDEGREE * 45, GEOM_UNITSINDEGREE * -60, 10));
			projection.Project(Vector3i(lon, lat, 10), Vector3i(GEOM_UNITSINDEGREE * -45, GEOM_UNITSINDEGREE * 60, 10));
			count += 4;
		}
	}
	gettimeofday(&end, NULL);

	float sec = (float)(end.tv_sec - start.tv_sec) + (float)(end.tv_usec - start.tv_usec)/1000000.0f;

	fprintf(stderr, "  %d points, %f seconds, %f points per second\n", count, sec, (float)count/sec);
}

int main() {
	fprintf(stderr, "MercatorProjection:\n");
	ProjBench(MercatorProjection());

	fprintf(stderr, "SphericalProjection:\n");
	ProjBench(SphericalProjection());

	return 0;
}
