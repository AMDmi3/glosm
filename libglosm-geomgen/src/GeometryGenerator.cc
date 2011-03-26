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

#include <glosm/GeometryGenerator.hh>

#include <glosm/OsmDatasource.hh>
#include <glosm/Geometry.hh>
#include <glosm/GeometryOperations.hh>
#include <glosm/MetricBasis.hh>
#include <glosm/geomath.h>

#include <list>
#include <cstdlib>
#include <cstdio>

typedef std::list<Vector2i> VertexList;
typedef std::vector<Vector2i> VertexVector;

static void CreateLines(Geometry& geom, const VertexVector& vertices, int z, const OsmDatasource::Way& /*unused*/) {
	geom.StartLine();
	for (unsigned int i = 0; i < vertices.size(); ++i)
		geom.AppendLine(Vector3i(vertices[i], z));
}

static void CreateVerticalLines(Geometry& geom, const VertexVector& vertices, int minz, int maxz, const OsmDatasource::Way& way) {
	for (unsigned int i = way.Closed ? 1 : 0; i < vertices.size(); ++i)
		geom.AddLine(Vector3i(vertices[i], minz), Vector3i(vertices[i], maxz));
}

static void CreateSmartVerticalLines(Geometry& geom, const VertexVector& vertices, int minz, int maxz, float minslope, const OsmDatasource::Way& way) {
	double cosminslope = cos(minslope/180.0*M_PI);

	if (vertices.size() < 2)
		return CreateVerticalLines(geom, vertices, minz, maxz, way);

	if (way.Closed) {
		for (unsigned int i = 0; i < vertices.size() - 1; ++i) {
			Vector3d to_prev = ToLocalMetric(vertices[i == 0 ? vertices.size() - 2 : i - 1], vertices[i]).Normalized();
			Vector3d to_next = ToLocalMetric(vertices[i + 1], vertices[i]).Normalized();

			if (fabs(to_prev.DotProduct(to_next)) < cosminslope)
				geom.AddLine(Vector3i(vertices[i], minz), Vector3i(vertices[i], maxz));
		}
	} else {
		for (unsigned int i = 1; i < vertices.size() - 1; ++i) {
			Vector3d to_prev = ToLocalMetric(vertices[i - 1], vertices[i]).Normalized();
			Vector3d to_next = ToLocalMetric(vertices[i + 1], vertices[i]).Normalized();

			if (fabs(to_prev.DotProduct(to_next)) < cosminslope)
				geom.AddLine(Vector3i(vertices[i], minz), Vector3i(vertices[i], maxz));
		}
		geom.AddLine(Vector3i(vertices.front(), minz), Vector3i(vertices.front(), maxz));
		geom.AddLine(Vector3i(vertices.back(), minz), Vector3i(vertices.back(), maxz));
	}
}

static void CreateWalls(Geometry& geom, const VertexVector& vertices, int minz, int maxz, const OsmDatasource::Way& /*unused*/) {
	for (unsigned int i = 1; i < vertices.size(); ++i)
		geom.AddQuad(Vector3i(vertices[i-1], minz), Vector3i(vertices[i-1], maxz), Vector3i(vertices[i], maxz), Vector3i(vertices[i], minz));
}

static void CreateWall(Geometry& geom, const VertexVector& vertices, int minz, int maxz, const OsmDatasource::Way& /*unused*/) {
	for (unsigned int i = 1; i < vertices.size(); ++i) {
		geom.AddQuad(Vector3i(vertices[i-1], minz), Vector3i(vertices[i-1], maxz), Vector3i(vertices[i], maxz), Vector3i(vertices[i], minz));
		geom.AddQuad(Vector3i(vertices[i-1], maxz), Vector3i(vertices[i-1], minz), Vector3i(vertices[i], minz), Vector3i(vertices[i], maxz));
	}
}

static void CreateArea(Geometry& geom, const VertexVector& vertices, bool revorder, int z, const OsmDatasource::Way& way) {
	if (vertices.size() < 3 || !way.Closed)
		return;

	VertexList vert;
	for (VertexVector::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
		vert.push_back(*i);
	vert.pop_back();

	VertexList::iterator i0, i1, i2, o;
	i0 = vert.begin();

	/* quick & dirty triangulation for roofs
	 * this should be rewritten to support sprips and/or fans,
	 * and there should be no possibility of infinite loop,
	 * as well as this iteration counter
	 */
	int iters = 1000;
	while (vert.size() >= 3) {
		if (++(i1 = i0) == vert.end())
			i1 = vert.begin();
		if (++(i2 = i1) == vert.end())
			i2 = vert.begin();

		osmlong_t area = 0;
		area += (osmlong_t)i0->x*(osmlong_t)i1->y - (osmlong_t)i0->y*(osmlong_t)i1->x;
		area += (osmlong_t)i1->x*(osmlong_t)i2->y - (osmlong_t)i1->y*(osmlong_t)i2->x;
		area += (osmlong_t)i2->x*(osmlong_t)i0->y - (osmlong_t)i2->y*(osmlong_t)i0->x;

		bool ok = true;
		o = i2;
		while (1) {
			if (++o == vert.end())
				o = vert.begin();

			if (o == i0)
				break;

			if (Vector2l(*o).IsInTriangle(*i0, *i1, *i2)) {
				ok = false;
				break;
			}
		}

		if ((area < 0) && ok) {
			if (revorder)
				geom.AddTriangle(Vector3i(*i0, z), Vector3i(*i1, z), Vector3i(*i2, z));
			else
				geom.AddTriangle(Vector3i(*i0, z), Vector3i(*i2, z), Vector3i(*i1, z));

			vert.erase(i1);
		}

		if (++i0 == vert.end())
			i0 = vert.begin();

		if (--iters == 0) {
			/* FIXME: add way ID here, lacks interface to datasource */
			fprintf(stderr, "warning: triangulation failed: giving up on %u/%u points\n", (unsigned int)vert.size(), (unsigned int)vertices.size());
			return;
		}
	}
}

static void CreateRoof(Geometry& geom, const VertexVector& vertices, int z, const OsmDatasource::Way& way) {
	float slope = 30.0;
	bool along = true;

	OsmDatasource::TagsMap::const_iterator shape, tag;

	if ((tag = way.Tags.find("building:roof:angle")) != way.Tags.end())
		slope = strtof(tag->second.c_str(), NULL);
	if ((tag = way.Tags.find("building:roof:orientation")) != way.Tags.end() && tag->second == "across")
		along = false;

	std::vector<Vector3i> vert;
	vert.reserve(vertices.size());
	for (VertexVector::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
		vert.push_back(Vector3i(*i, z));

	if (vert.size() > 3 && way.Closed &&
				(shape = way.Tags.find("building:roof:shape")) != way.Tags.end() &&
				(shape->second == "pyramidal" || shape->second == "conical")
			) {
		/* calculate center */
		Vector3l center;
		for (unsigned int i = 0; i < vert.size() - 1; i++)
			center += Vector3i(vert[i]);
		center /= vert.size() - 1;

		/* calculate mean face length */
		float facelength = 0.0;
		for (unsigned int i = 0; i < vert.size() - 1; i++)
			facelength += (ToLocalMetric(vert[i], center)/2.0 + ToLocalMetric(vert[i+1], center)/2.0).Length();
		facelength /= vert.size() - 1;

		center.z += (tan(slope/180.0*M_PI) * facelength) * GEOM_UNITSINMETER;

		for (unsigned int i = 0; i < vert.size() - 1; i++) {
			geom.AddTriangle(vert[i], center, vert[i+1]);
			geom.AddLine(vert[i], center);
		}
		return;
	}

	/* only 4-vert buildings are supported for other types, yet */
	if (vert.size() == 5 && way.Closed && (shape = way.Tags.find("building:roof:shape")) != way.Tags.end()) {
		float length1 = ToLocalMetric(vert[0], vert[1]).Length();
		float length2 = ToLocalMetric(vert[1], vert[2]).Length();

		if (shape->second == "pyramidal") {
			Vector3i center = ((Vector3l)vert[0] + (Vector3l)vert[1] + (Vector3l)vert[2] + (Vector3l)vert[3]) / 4;
			center.z += (tan(slope/180.0*M_PI) * std::min(length1, length2) * 0.5) * GEOM_UNITSINMETER;

			for (int i = 0; i < 4; i++) {
				geom.AddTriangle(vert[i], center, vert[i+1]);
				geom.AddLine(vert[i], center);
			}
			return;
		} else if (shape->second == "pitched") {
			if (!!(length1 < length2) ^ !along) {
				osmint_t height = (tan(slope/180.0*M_PI) * length1 * 0.5) * GEOM_UNITSINMETER;

				Vector3i center1 = ((Vector3l)vert[0] + (Vector3l)vert[1])/2;
				Vector3i center2 = ((Vector3l)vert[2] + (Vector3l)vert[3])/2;

				center1.z += height;
				center2.z += height;

				geom.AddTriangle(vert[0], center1, vert[1]);
				geom.AddTriangle(vert[2], center2, vert[3]);
				geom.AddQuad(vert[2], vert[1], center1, center2);
				geom.AddQuad(vert[0], vert[3], center2, center1);

				geom.AddLine(vert[0], center1); geom.AddLine(center1, vert[1]);
				geom.AddLine(vert[2], center2); geom.AddLine(center2, vert[3]);
				geom.AddLine(center1, center2);
			} else {
				osmint_t height = (tan(slope/180.0*M_PI) * length2 * 0.5) * GEOM_UNITSINMETER;

				Vector3i center1 = ((Vector3l)vert[1] + (Vector3l)vert[2])/2;
				Vector3i center2 = ((Vector3l)vert[0] + (Vector3l)vert[3])/2;

				center1.z += height;
				center2.z += height;

				geom.AddTriangle(vert[1], center1, vert[2]);
				geom.AddTriangle(vert[3], center2, vert[0]);
				geom.AddQuad(vert[1], vert[0], center2, center1);
				geom.AddQuad(vert[3], vert[2], center1, center2);

				geom.AddLine(vert[1], center1); geom.AddLine(center1, vert[2]);
				geom.AddLine(vert[3], center2); geom.AddLine(center2, vert[0]);
				geom.AddLine(center1, center2);
			}
			return;
		} else if (shape->second == "hipped") {
			if (length1 < length2) {
				osmint_t height = (tan(slope/180.0*M_PI) * length1 * 0.5) * GEOM_UNITSINMETER;

				Vector3i center1 = ((Vector3l)vert[0] + (Vector3l)vert[1])/2;
				Vector3i center2 = ((Vector3l)vert[2] + (Vector3l)vert[3])/2;

				Vector3i delta = FromLocalMetric(ToLocalMetric(center2, center1).Normalized() * ToLocalMetric(vert[0], center1).Length(), center1) - center1;

				center1 += delta;
				center2 -= delta;

				center1.z += height;
				center2.z += height;

				geom.AddTriangle(vert[0], center1, vert[1]);
				geom.AddTriangle(vert[2], center2, vert[3]);
				geom.AddQuad(vert[2], vert[1], center1, center2);
				geom.AddQuad(vert[0], vert[3], center2, center1);

				geom.AddLine(vert[0], center1); geom.AddLine(center1, vert[1]);
				geom.AddLine(vert[2], center2); geom.AddLine(center2, vert[3]);
				geom.AddLine(center1, center2);
			} else {
				osmint_t height = (tan(slope/180.0*M_PI) * length2 * 0.5) * GEOM_UNITSINMETER;

				Vector3i center1 = ((Vector3l)vert[1] + (Vector3l)vert[2])/2;
				Vector3i center2 = ((Vector3l)vert[0] + (Vector3l)vert[3])/2;

				Vector3i delta = FromLocalMetric(ToLocalMetric(center2, center1).Normalized() * ToLocalMetric(vert[1], center1).Length(), center1) - center1;

				center1 += delta;
				center2 -= delta;

				center1.z += height;
				center2.z += height;

				geom.AddTriangle(vert[1], center1, vert[2]);
				geom.AddTriangle(vert[3], center2, vert[0]);
				geom.AddQuad(vert[1], vert[0], center2, center1);
				geom.AddQuad(vert[3], vert[2], center1, center2);

				geom.AddLine(vert[1], center1); geom.AddLine(center1, vert[2]);
				geom.AddLine(vert[3], center2); geom.AddLine(center2, vert[0]);
				geom.AddLine(center1, center2);
			}
			return;
		} else if (shape->second == "crosspitched") {
			int height = (tan(slope/180.0*M_PI) * std::min(length1, length2) * 0.5) * GEOM_UNITSINMETER;

			Vector3i center = ((Vector3l)vert[0] + (Vector3l)vert[1] + (Vector3l)vert[2] + (Vector3l)vert[3]) / 4;
			center.z += height;

			for (int i = 0; i < 4; ++i) {
				Vector3i sidecenter = ((Vector3l)vert[i] + (Vector3l)vert[i+1]) / 2;
				sidecenter.z += height;

				geom.AddTriangle(vert[i], center, sidecenter);
				geom.AddTriangle(center, vert[i+1], sidecenter);
				geom.AddTriangle(vert[i+1], vert[i], sidecenter);

				geom.AddLine(sidecenter, center);
				geom.AddLine(sidecenter, vert[i]);
				geom.AddLine(sidecenter, vert[i+1]);
				geom.AddLine(vert[i], center);
			}
			return;
		}
	}

	/* fallback - flat roof */
	return CreateArea(geom, vertices, false, z, way);
}

static void CreateRoad(Geometry& geom, const VertexVector& vertices, float width, const OsmDatasource::Way& /*unused*/) {
	if (vertices.size() < 2)
		return;

	VertexVector::const_iterator prev = vertices.end();
	VertexVector::const_iterator next;
	Vector2i prev_points[2];
	Vector2i new_points[2];
	for (VertexVector::const_iterator i = vertices.begin(); i != vertices.end(); i++) {
		++(next = i);

		Vector3d to_prev, to_next;

		if (next != vertices.end()) {
			to_next = ToLocalMetric(*next, *i).Normalized();
			if (prev != vertices.end())
				to_prev = ToLocalMetric(*prev, *i).Normalized();
			else
				to_prev = -to_next;
		} else {
			to_prev = ToLocalMetric(*prev, *i).Normalized();
			to_next = -to_prev;
		}

		Vector3d normside = to_next.CrossProduct(Vector3d(0.0, 0.0, 1.0));
		Vector3d side = normside * width / 2.0;

		Vector3d bisect = to_prev + to_next;
		double cosangle = to_next.DotProduct(to_prev);

		if (cosangle > 0.99) {
			/* FIXME: add way ID here, lacks interface to datasource */
			fprintf(stderr, "warning: too sharp road turn (cos=%f), likely data error\n", cosangle);
			prev = i;
			continue;
		}

		if (bisect.Length() < 0.001) { /* constant == sin(alpha/2), where alpha is minimal angle to consider joint non-straight */
			/* almost straight segment, just use normals */
			new_points[0] = FromLocalMetric(-side, *i);
			new_points[1] = FromLocalMetric(side, *i);
		} else {
			bisect = bisect.Normalized() * (width / 2.0) / sin(acos(cosangle) / 2.0);

			if (bisect.Normalized().DotProduct(normside) < 0.0) {
				new_points[0] = FromLocalMetric(bisect, *i);
				new_points[1] = FromLocalMetric(-bisect, *i);
			} else {
				new_points[0] = FromLocalMetric(-bisect, *i);
				new_points[1] = FromLocalMetric(bisect, *i);
			}
		}

		if (prev != vertices.end())
			geom.AddQuad(prev_points[0], prev_points[1], new_points[1], new_points[0]);

		prev_points[0] = new_points[0];
		prev_points[1] = new_points[1];

		prev = i;
	}
}

static void CreatePowerTower(Geometry& geom, const Vector3i& pos, const Vector3d& side) {
	MetricBasis b(pos, side);

	float w1 = 1.05;
	float w2 = 0.5;
	float h1 = 11.0;
	float h2 = 21.0;
	float h3 = 23.0;

	/* lower section */
	geom.AddQuad(b.Get(w1, w1, 0.0), b.Get(-w1, w1, 0.0), b.Get(-w2, w2, h1), b.Get(w2, w2, h1));
	geom.AddQuad(b.Get(-w1, w1, 0.0), b.Get(-w1, -w1, 0.0), b.Get(-w2, -w2, h1), b.Get(-w2, w2, h1));
	geom.AddQuad(b.Get(-w1, -w1, 0.0), b.Get(w1, -w1, 0.0), b.Get(w2, -w2, h1), b.Get(-w2, -w2, h1));
	geom.AddQuad(b.Get(w1, -w1, 0.0), b.Get(w1, w1, 0.0), b.Get(w2, w2, h1), b.Get(w2, -w2, h1));

	geom.AddLine(b.Get(w1, w1, 0.0), b.Get(-w1, w1, 0.0));
	geom.AddLine(b.Get(-w1, w1, 0.0), b.Get(-w1, -w1, 0.0));
	geom.AddLine(b.Get(-w1, -w1, 0.0), b.Get(w1, -w1, 0.0));
	geom.AddLine(b.Get(w1, -w1, 0.0), b.Get(w1, w1, 0.0));

	geom.AddLine(b.Get(-w1, w1, 0.0), b.Get(-w2, w2, h1));
	geom.AddLine(b.Get(-w1, -w1, 0.0), b.Get(-w2, -w2, h1));
	geom.AddLine(b.Get(w1, -w1, 0.0), b.Get(w2, -w2, h1));
	geom.AddLine(b.Get(w1, w1, 0.0), b.Get(w2, w2, h1));

	/* middle section */
	geom.AddQuad(b.Get(w2, w2, h1), b.Get(-w2, w2, h1), b.Get(-w2, w2, h2), b.Get(w2, w2, h2));
	geom.AddQuad(b.Get(-w2, w2, h1), b.Get(-w2, -w2, h1), b.Get(-w2, -w2, h2), b.Get(-w2, w2, h2));
	geom.AddQuad(b.Get(-w2, -w2, h1), b.Get(w2, -w2, h1), b.Get(w2, -w2, h2), b.Get(-w2, -w2, h2));
	geom.AddQuad(b.Get(w2, -w2, h1), b.Get(w2, w2, h1), b.Get(w2, w2, h2), b.Get(w2, -w2, h2));

	geom.AddLine(b.Get(w2, w2, h1), b.Get(-w2, w2, h1));
	geom.AddLine(b.Get(-w2, w2, h1), b.Get(-w2, -w2, h1));
	geom.AddLine(b.Get(-w2, -w2, h1), b.Get(w2, -w2, h1));
	geom.AddLine(b.Get(w2, -w2, h1), b.Get(w2, w2, h1));

	geom.AddLine(b.Get(-w2, w2, h1), b.Get(-w2, w2, h2));
	geom.AddLine(b.Get(-w2, -w2, h1), b.Get(-w2, -w2, h2));
	geom.AddLine(b.Get(w2, -w2, h1), b.Get(w2, -w2, h2));
	geom.AddLine(b.Get(w2, w2, h1), b.Get(w2, w2, h2));

	/* top section */
	geom.AddTriangle(b.Get(w2, w2, h2), b.Get(-w2, w2, h2), b.Get(0, 0, h3));
	geom.AddTriangle(b.Get(-w2, w2, h2), b.Get(-w2, -w2, h2), b.Get(0, 0, h3));
	geom.AddTriangle(b.Get(-w2, -w2, h2), b.Get(w2, -w2, h2), b.Get(0, 0, h3));
	geom.AddTriangle(b.Get(w2, -w2, h2), b.Get(w2, w2, h2), b.Get(0, 0, h3));

	geom.AddLine(b.Get(w2, w2, h2), b.Get(-w2, w2, h2));
//	geom.AddLine(b.Get(-w2, w2, h2), b.Get(-w2, -w2, h2));
	geom.AddLine(b.Get(-w2, -w2, h2), b.Get(w2, -w2, h2));
//	geom.AddLine(b.Get(w2, -w2, h2), b.Get(w2, w2, h2));

	geom.AddLine(b.Get(w2, w2, h2), b.Get(0, 0, h3));
	geom.AddLine(b.Get(-w2, w2, h2), b.Get(0, 0, h3));
	geom.AddLine(b.Get(-w2, -w2, h2), b.Get(0, 0, h3));
	geom.AddLine(b.Get(w2, -w2, h2), b.Get(0, 0, h3));

	/* arms */
	for (int h = 14; h <= 21; h += 3) {
		float l = (h == 17) ? 3.3 : 2.0;
		geom.AddTriangle(b.Get(w2, -w2, h), b.Get(w2, w2, h), b.Get(l, 0.0, h));
		geom.AddTriangle(b.Get(w2, -w2, h+1), b.Get(w2, -w2, h), b.Get(l, 0.0, h));
		geom.AddTriangle(b.Get(w2, w2, h+1), b.Get(w2, -w2, h+1), b.Get(l, 0.0, h));
		geom.AddTriangle(b.Get(w2, w2, h), b.Get(w2, w2, h+1), b.Get(l, 0.0, h));

		geom.AddLine(b.Get(w2, -w2, h), b.Get(w2, w2, h));
		geom.AddLine(b.Get(w2, -w2, h+1), b.Get(w2, w2, h+1));

		geom.AddLine(b.Get(w2, w2, h), b.Get(l, 0.0, h));
		geom.AddLine(b.Get(w2, -w2, h), b.Get(l, 0.0, h));
		geom.AddLine(b.Get(w2, w2, h+1), b.Get(l, 0.0, h));
		geom.AddLine(b.Get(w2, -w2, h+1), b.Get(l, 0.0, h));

		geom.AddTriangle(b.Get(-w2, w2, h), b.Get(-w2, -w2, h), b.Get(-l, 0.0, h));
		geom.AddTriangle(b.Get(-w2, -w2, h), b.Get(-w2, -w2, h+1), b.Get(-l, 0.0, h));
		geom.AddTriangle(b.Get(-w2, -w2, h+1), b.Get(-w2, w2, h+1), b.Get(-l, 0.0, h));
		geom.AddTriangle(b.Get(-w2, w2, h+1), b.Get(-w2, w2, h), b.Get(-l, 0.0, h));

		geom.AddLine(b.Get(-w2, w2, h), b.Get(-l, 0.0, h));
		geom.AddLine(b.Get(-w2, -w2, h), b.Get(-l, 0.0, h));
		geom.AddLine(b.Get(-w2, w2, h+1), b.Get(-l, 0.0, h));
		geom.AddLine(b.Get(-w2, -w2, h+1), b.Get(-l, 0.0, h));

		geom.AddLine(b.Get(-w2, -w2, h), b.Get(-w2, w2, h));
		geom.AddLine(b.Get(-w2, -w2, h+1), b.Get(-w2, w2, h+1));
	}
}

static void CreatePhysicalLine(Geometry& geom, const Vector3i& one, const Vector3i& two, float radius) {
	Vector3d side = ToLocalMetric(one, two).Normalized().CrossProduct(Vector3d(0.0, 0.0, 1.0)) * radius;
	Vector3d up = Vector3d(0.0, 0.0, radius);

	geom.AddQuad(FromLocalMetric(-up, one), FromLocalMetric(-up, two), FromLocalMetric(-side, two), FromLocalMetric(-side, one));
	geom.AddQuad(FromLocalMetric(-side, one), FromLocalMetric(-side, two), FromLocalMetric(up, two), FromLocalMetric(up, one));
	geom.AddQuad(FromLocalMetric(up, one), FromLocalMetric(up, two), FromLocalMetric(side, two), FromLocalMetric(side, one));
	geom.AddQuad(FromLocalMetric(side, one), FromLocalMetric(side, two), FromLocalMetric(-up, two), FromLocalMetric(-up, one));
}

template<class T>
static inline T catenary(T x, T a) {
	return a * cosh(x/a);
}

static void CreateWire(Geometry& geom, const Vector3i& one, const Vector3i& two) {
	/* @todo take wire diameter from tags; for now 8cm is used which is common for 35kV */
	float length = ToLocalMetric(two, one).Length();

	/* catenary argument autotuning:
	 * since droop(a) = (cat(0, a) - cat(1, a)) = a - a*cosh(1/a) -> -1/(2a)
	 * we calculate a for desired droop ratio which will be accurate for long
	 * wires, and we set a lower cap on it so short wires look good as well */
	float a = length / (2.0 * 2.0 * 5.0); /* 5 is desired droop in meters */
	if (a < 4.0) a = 4.0;
	float dh = catenary(1.0f, a); /* catenary fix so delta equals to 0 in -1 and 1 */

	int sections = 8;
	float prevh = 0.0;

	/* render catenary */
	for (int node = 1; node <= sections; ++node) {
		float h = (catenary((float)node/(float)sections * 2.0f - 1.0f, a) - dh) * length / 2.0f;
		CreatePhysicalLine(geom,
				FromLocalMetric(ToLocalMetric(two, one) * (double)(node-1)/(double)(sections) + Vector3d(0.0, 0.0, prevh), one),
				FromLocalMetric(ToLocalMetric(two, one) * (double)(node)/(double)(sections) + Vector3d(0.0, 0.0, h), one),
				0.08/2
			);
		prevh = h;
	}
}

static void CreatePowerLine(Geometry& geom, const VertexVector& vertices, const OsmDatasource::Way& /*unused*/) {
	if (vertices.size() < 2)
		return;

	Vector3d prev_side, to_prev, to_next, side;
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		if (i != vertices.size() - 1)
			to_next = ToLocalMetric(vertices[i+1], vertices[i]).Normalized();
		to_prev = i == 0 ? -to_next : ToLocalMetric(vertices[i-1], vertices[i]).Normalized();
		if (i == vertices.size() - 1)
			to_next = -to_prev;

		side = (to_next == to_prev) ? to_next : (to_next - to_prev).Normalized().CrossProduct(Vector3d(0.0, 0.0, 1.0));

		if (i != 0) {
			CreateWire(geom, FromLocalMetric(Vector3d(prev_side*2.0, 14.0-0.5), vertices[i-1]), FromLocalMetric(Vector3d(side*2.0, 14.0-0.5), vertices[i]));
			CreateWire(geom, FromLocalMetric(Vector3d(prev_side*3.3, 17.0-0.5), vertices[i-1]), FromLocalMetric(Vector3d(side*3.3, 17.0-0.5), vertices[i]));
			CreateWire(geom, FromLocalMetric(Vector3d(prev_side*2.0, 20.0-0.5), vertices[i-1]), FromLocalMetric(Vector3d(side*2.0, 20.0-0.5), vertices[i]));
			CreateWire(geom, FromLocalMetric(Vector3d(-prev_side*2.0, 14.0-0.5), vertices[i-1]), FromLocalMetric(Vector3d(-side*2.0, 14.0-0.5), vertices[i]));
			CreateWire(geom, FromLocalMetric(Vector3d(-prev_side*3.3, 17.0-0.5), vertices[i-1]), FromLocalMetric(Vector3d(-side*3.3, 17.0-0.5), vertices[i]));
			CreateWire(geom, FromLocalMetric(Vector3d(-prev_side*2.0, 20.0-0.5), vertices[i-1]), FromLocalMetric(Vector3d(-side*2.0, 20.0-0.5), vertices[i]));
			CreateWire(geom, FromLocalMetric(Vector3d(0.0, 0.0, 23.0), vertices[i-1]), FromLocalMetric(Vector3d(0.0, 0.0, 23.0), vertices[i]));
		}

		/* Placeholder "tower" until models are added */
		CreatePowerTower(geom, vertices[i], side);

		prev_side = side;
	}
}

static float GetMaxHeight(const OsmDatasource::Way& way) {
	OsmDatasource::TagsMap::const_iterator building, tag;

	if ((tag = way.Tags.find("building:part:height")) != way.Tags.end()) {
		/* building:part:height is topmost precedence (hack for Ostankino tower) */
		return strtof(tag->second.c_str(), NULL);
	} else if ((tag = way.Tags.find("height")) != way.Tags.end()) {
		/* explicit height - topmost precedence in all other cases */
		return strtof(tag->second.c_str(), NULL);
	} else if ((tag = way.Tags.find("building:levels")) != way.Tags.end()) {
		/* count level heights as 3 meters */
		int levels = strtol(tag->second.c_str(), NULL, 10);
		float h = 3.0 * levels;

		/* also add 1 meter for basement for short buildings
		 * (except for garages which doesn't have one) - should work
		 * well in rural areas */
		if (levels == 1 && (building = way.Tags.find("building")) != way.Tags.end() && building->second != "garages" && building->second != "garage")
			h += 1.0;

		return h;
	}

	return 0.0;
}

static float GetMinHeight(const OsmDatasource::Way& way) {
	OsmDatasource::TagsMap::const_iterator building, tag, tag1;

	if ((tag = way.Tags.find("min_height")) != way.Tags.end()) {
		/* explicit height - topmost precedence in all other cases */
		return strtof(tag->second.c_str(), NULL);
	} else if ((tag = way.Tags.find("building:min_level")) != way.Tags.end() || (tag = way.Tags.find("building:skipped_levels")) != way.Tags.end()) {
		/* count level heights as 3 meters */
		float h = 3.0 * strtol(tag->second.c_str(), NULL, 10);

		/* in building:min_level scheme, levels are counted from zero, which may be fixed by building:ground_level... */
		if (tag->first == "building:min_level" && (tag1 = way.Tags.find("building:ground_level")) != way.Tags.end())
			h -= 3.0 * strtol(tag1->second.c_str(), NULL, 10);
		/* ...while in my proposal (building:skipped_levels) everythng just works */

		return h;
	}

	return 0.0;
}

static int GetHighwayLanes(const std::string& highway, const OsmDatasource::Way& way) {
	OsmDatasource::TagsMap::const_iterator tag;

	/* explicitely tagged lanes have top */
	if ((tag = way.Tags.find("lanes")) != way.Tags.end())
		return strtol(tag->second.c_str(), NULL, 10);

	bool oneway = false;
	if ((tag = way.Tags.find("oneway")) != way.Tags.end() && tag->second != "no")
		oneway = true;

	/* motorway assumes one-way */
	if (highway == "motorway" || highway == "motorway_link")
		oneway = true;

	if (highway == "service" || highway == "track") {
		return 1;
	} else if (highway == "residential") {
		return 2;
	} else {
		return oneway ? 2 : 4;
	}
}

static float GetHighwayWidth(const std::string& highway, const OsmDatasource::Way& way) {
	OsmDatasource::TagsMap::const_iterator tag;

	/* explicitely tagged lanes have top */
	if ((tag = way.Tags.find("width")) != way.Tags.end())
		return strtof(tag->second.c_str(), NULL);

	if (highway == "path") {
		return 0.5f;
	} else if (highway == "footway" || highway == "steps") {
		return 2.0f;
	} else if (highway == "pedestrian") {
		return 3.0f;
	} else {
		return GetHighwayLanes(highway, way) * 3.5f; /* likely 4 is closer to truth */
	}
}

static void WayDispatcher(Geometry& geom, const OsmDatasource& datasource, int flags, const OsmDatasource::Way& way) {
	osmint_t minz = GetMinHeight(way) * GEOM_UNITSINMETER;
	osmint_t maxz = GetMaxHeight(way) * GEOM_UNITSINMETER;

	if (minz < 0)
		minz = 0;
	if (maxz < minz) {
		fprintf(stderr, "warning: max height < min height for object\n");
		maxz = minz = 0;
	}

	OsmDatasource::TagsMap::const_iterator t;

	VertexVector vertices;
	vertices.reserve(way.Nodes.size());

	if (way.Clockwise)
		for (OsmDatasource::Way::NodesList::const_iterator n = way.Nodes.begin(); n != way.Nodes.end(); ++n)
			vertices.push_back(datasource.GetNode(*n).Pos);
	else
		for (OsmDatasource::Way::NodesList::const_reverse_iterator n = way.Nodes.rbegin(); n != way.Nodes.rend(); ++n)
			vertices.push_back(datasource.GetNode(*n).Pos);

	/* dispatch */
	if ((way.Tags.find("building") != way.Tags.end() || way.Tags.find("building:part") != way.Tags.end()) && minz != maxz) {
		if (flags & GeometryDatasource::DETAIL) {
			CreateWalls(geom, vertices, minz, maxz, way);
			CreateRoof(geom, vertices, maxz, way);

			CreateLines(geom, vertices, minz, way);
			CreateLines(geom, vertices, maxz, way);
			CreateSmartVerticalLines(geom, vertices, minz, maxz, 5.0, way);

			if (minz > 1) {
				CreateArea(geom, vertices, true, minz, way);
				CreateLines(geom, vertices, minz, way);
			}
		}
	} else if ((t = way.Tags.find("man_made")) != way.Tags.end() && (t->second == "tower" || t->second == "chimney") && minz != maxz) {
		if (flags & GeometryDatasource::DETAIL) {
			CreateWalls(geom, vertices, minz, maxz, way);
			CreateArea(geom, vertices, false, maxz, way);

			CreateLines(geom, vertices, minz, way);
			CreateLines(geom, vertices, maxz, way);
			CreateSmartVerticalLines(geom, vertices, minz, maxz, 5.0, way);
		}
	} else if (way.Tags.find("barrier") != way.Tags.end()) {
		if (flags & GeometryDatasource::DETAIL) {
			if (maxz == minz)
				maxz += 2 * GEOM_UNITSINMETER;
			CreateWall(geom, vertices, minz, maxz, way);

			CreateLines(geom, vertices, minz, way);
			CreateLines(geom, vertices, maxz, way);
			CreateVerticalLines(geom, vertices, minz, maxz, way);
		}
	} else if ((t = way.Tags.find("highway")) != way.Tags.end()) {
		if (flags & GeometryDatasource::DETAIL) {
			OsmDatasource::TagsMap::const_iterator t1;

			if ((t1 = way.Tags.find("area")) != way.Tags.end() && t1->second != "no") {
				/* area */
				CreateArea(geom, vertices, false, 0, way);
			} else {
				CreateRoad(geom, vertices, GetHighwayWidth(t->second, way), way);
			}
		} else if ((flags & GeometryDatasource::GROUND) && (
				t->second == "motorway" || t->second == "motorway_link" ||
				t->second == "trunk" || t->second == "trunk_link" ||
				t->second == "primary" || t->second == "primary_link" ||
				t->second == "secondary" || t->second == "secondary_link" ||
				t->second == "tertiary")) {
			CreateLines(geom, vertices, minz, way);
		}
	} else if ((t = way.Tags.find("railway")) != way.Tags.end() && (t->second == "rail")) {
		if (flags & GeometryDatasource::DETAIL) {
			CreateLines(geom, vertices, minz, way);
		} else if (flags & GeometryDatasource::GROUND) {
			if (t->second == "rail")
				CreateLines(geom, vertices, minz, way);
		}
	} else if ((t = way.Tags.find("boundary")) != way.Tags.end() && (t->second == "administrative")) {
		if (flags & GeometryDatasource::GROUND)
			CreateLines(geom, vertices, minz, way);
	} else if ((t = way.Tags.find("waterway")) != way.Tags.end()) {
		if (flags & GeometryDatasource::GROUND)
			CreateLines(geom, vertices, minz, way);
	} else if ((t = way.Tags.find("natural")) != way.Tags.end()) {
		if (flags & GeometryDatasource::GROUND)
			CreateLines(geom, vertices, minz, way);
	} else if ((t = way.Tags.find("landuse")) != way.Tags.end()) {
		if (flags & GeometryDatasource::GROUND)
			CreateLines(geom, vertices, minz, way);
	} else if ((t = way.Tags.find("power")) != way.Tags.end() && (t->second == "line")) {
		if (flags & GeometryDatasource::DETAIL)
			CreatePowerLine(geom, vertices, way);
	} else {
		if (flags & GeometryDatasource::DETAIL)
			CreateLines(geom, vertices, minz, way);
	}
}

GeometryGenerator::GeometryGenerator(const OsmDatasource& datasource) : datasource_(datasource) {
}

void GeometryGenerator::GetGeometry(Geometry& geom, const BBoxi& bbox, int flags) const {
	std::vector<OsmDatasource::Way> ways;

	/* safe bbox is a bit wider than requested one to be sure
	 * all ways are included, even those which have width */
	float extra_width = 24.0; /* still may be not sufficient, e.g. very wide roads */
	BBoxi safe_bbox = BBoxi(
			FromLocalMetric(-Vector2d(extra_width, extra_width), bbox.GetBottomLeft()),
			FromLocalMetric(Vector2d(extra_width, extra_width), bbox.GetTopRight())
		);
	datasource_.GetWays(ways, safe_bbox);

	Geometry temp;

	for (std::vector<OsmDatasource::Way>::const_iterator w = ways.begin(); w != ways.end(); ++w)
		WayDispatcher(temp, datasource_, flags, *w);

	geom.AppendCropped(temp, bbox);
}

Vector2i GeometryGenerator::GetCenter() const {
	return datasource_.GetCenter();
}

BBoxi GeometryGenerator::GetBBox() const {
	return datasource_.GetBBox();
}
