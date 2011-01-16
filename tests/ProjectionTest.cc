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

/*
 * This test projects a point using Mercator projection, then
 * unprojects it. Result of project/unproject should match the
 * original point if the point is not far from origin.
 */

#include <stdio.h>
#include <limits>
#include <stdlib.h>

#include "MercatorProjection.hh"

int ProjTest(Projection projection) {
	osmint_t xref[] = { -1350000000, -450000000, 0, 450000000, 1350000000 };
	osmint_t yref[] = { -700000000, -450000000, 0, 450000000, 700000000 };

	Vector3i deltas[] = {
		Vector3i(0, 0, 0),

		Vector3i(1, 1, 10000),
		Vector3i(1, -1, 100000),
		Vector3i(-1, -1, 100000),
		Vector3i(-1, 1, 100000),

		Vector3i(1000, 1000, 10000),
		Vector3i(1000, -1000, 100000),
		Vector3i(-1000, -1000, 100000),
		Vector3i(-1000, 1000, 100000),

		/* +/- 100km on equator; if there's no error within this range, we can safely do Z8 geom tiles */
		Vector3i(9000000, 9000000, 10000),
		Vector3i(9000000, -9000000, 10000),
		Vector3i(-9000000, -9000000, 10000),
		Vector3i(-9000000, 9000000, 10000),
	};

	bool result = 0;

	for (unsigned int ixref = 0; ixref < sizeof(xref)/sizeof(xref[0]); ++ixref) {
		for (unsigned int iyref = 0; iyref < sizeof(yref)/sizeof(yref[0]); ++iyref) {
			for (unsigned int idelta = 0; idelta < sizeof(deltas)/sizeof(deltas[0]); ++idelta) {
				Vector3i ref = Vector3i(xref[ixref], yref[iyref], 0);
				Vector3i orig = ref + deltas[idelta];

				Vector3f projected = projection.Project(orig, ref);
				Vector3i unprojected = projection.UnProject(projected, ref);

				Vector3i error = unprojected - orig;
				error.x = abs(error.x);
				error.y = abs(error.y);
				error.z = abs(error.z);

				int maxerror = std::max(error.x, std::max(error.y, error.z));

				printf("[%d,%d,%d] by [%d,%d,%d] -> [%.10f,%.10f,%.10f] -> [%d,%d,%d] error=%d\n",
						orig.x, orig.y, orig.z,
						ref.x, ref.y, ref.z,
						projected.x, projected.y, projected.z,
						unprojected.x, unprojected.y, unprojected.z,
						maxerror
					);

				if (maxerror > 0)
					result = 1;
			}
		}
	}

	return result;
}

int main() {
	int result = 0;

	result |= ProjTest(MercatorProjection());

	return result;
}
