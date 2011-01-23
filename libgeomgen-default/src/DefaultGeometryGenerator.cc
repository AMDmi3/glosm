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

#include <glosm/DefaultGeometryGenerator.hh>

#include <glosm/OsmDatasource.hh>
#include <glosm/Geometry.hh>
#include <glosm/GeometryOperations.hh>

#include <glosm/geomath.h>

#include <stdio.h>
#include <stdlib.h>

#include <list>
#include <cassert>

typedef std::list<Vector2i> VertexList;
typedef std::vector<Vector2i> VertexVector;

static void CreateLines(Geometry& geom, const VertexList& vertices, int z, const OsmDatasource::Way& way) {
	if (vertices.size() < 2)
		return;

	VertexList::const_iterator prev = vertices.begin();
	for (VertexList::const_iterator i = ++(vertices.begin()); i != vertices.end(); i++) {
		geom.AddLine(Vector3i(*prev, z), Vector3i(*i, z));
		prev = i;
	}
}

static void CreateVerticalLines(Geometry& geom, const VertexList& vertices, int minz, int maxz, const OsmDatasource::Way& way) {
	VertexList::const_iterator i = vertices.begin();
	if (way.Closed)
		++i; /* avoid duplicate line */

	for (; i != vertices.end(); i++)
		geom.AddLine(Vector3i(*i, minz), Vector3i(*i, maxz));
}

static void CreateSmartVerticalLines(Geometry& geom, const VertexList& vertices, int minz, int maxz, float minslope, const OsmDatasource::Way& way) {
	VertexList::const_iterator i, prev, next;

	double cosminslope = cos(minslope/180.0*M_PI);

	if (vertices.size() < 2)
		return CreateVerticalLines(geom, vertices, minz, maxz, way);

	if (way.Closed) {
		prev = vertices.begin();
		++(i = prev);
		++(next = i);
	} else {
		prev = vertices.end();
		i = vertices.begin();
		++(next) = i;
	}

	for (; i != vertices.end(); i++, next++, prev++) {
		if (next == vertices.end() && way.Closed)
			++(++next); /* rewind to second point, as first is the same as last */

		if (next != vertices.end() && prev != vertices.end()) {
			Vector3d to_prev = ToLocalMetric(*prev, *i).Normalized();
			Vector3d to_next = ToLocalMetric(*next, *i).Normalized();
			if (fabs(to_prev.DotProduct(to_next)) > cosminslope)
				continue;
		}

		geom.AddLine(Vector3i(*i, minz), Vector3i(*i, maxz));
	}
}

static void CreateWalls(Geometry& geom, const VertexList& vertices, int minz, int maxz, const OsmDatasource::Way& way) {
	if (vertices.size() < 2 || !way.Closed)
		return;

	VertexList::const_iterator prev = vertices.begin();
	for (VertexList::const_iterator i = ++(vertices.begin()); i != vertices.end(); i++) {
		if (way.Clockwise)
			geom.AddQuad(Vector3i(*prev, minz), Vector3i(*prev, maxz), Vector3i(*i, maxz), Vector3i(*i, minz));
		else
			geom.AddQuad(Vector3i(*prev, maxz), Vector3i(*prev, minz), Vector3i(*i, minz), Vector3i(*i, maxz));

		prev = i;
	}
}

static void CreateWall(Geometry& geom, const VertexList& vertices, int minz, int maxz, const OsmDatasource::Way& way) {
	if (vertices.size() < 2)
		return;

	VertexList::const_iterator prev = vertices.begin();
	for (VertexList::const_iterator i = ++(vertices.begin()); i != vertices.end(); i++) {
		geom.AddQuad(Vector3i(*prev, minz), Vector3i(*prev, maxz), Vector3i(*i, maxz), Vector3i(*i, minz));
		geom.AddQuad(Vector3i(*prev, maxz), Vector3i(*prev, minz), Vector3i(*i, minz), Vector3i(*i, maxz));
		prev = i;
	}
}

static void CreateArea(Geometry& geom, const VertexList& vertices, bool revorder, int z, const OsmDatasource::Way& way) {
	if (vertices.size() < 3 || !way.Closed)
		return;

	VertexList vert = vertices;
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
			++i1;
		if (++(i2 = i1) == vert.end())
			++i2;

		osmlong_t area = 0;
		area += (osmlong_t)i0->x*(osmlong_t)i1->y - (osmlong_t)i0->y*(osmlong_t)i1->x;
		area += (osmlong_t)i1->x*(osmlong_t)i2->y - (osmlong_t)i1->y*(osmlong_t)i2->x;
		area += (osmlong_t)i2->x*(osmlong_t)i0->y - (osmlong_t)i2->y*(osmlong_t)i0->x;

		bool ok = true;
		for (++(o = i2); o != i0; ++o) {
			if (o != vert.end() && Vector2l(*o).IsInTriangle(*i0, *i1, *i2)) {
				ok = false;
				break;
			}
		}

		if ((area < 0) == way.Clockwise && ok) {
			if (!!way.Clockwise ^ !revorder)
				geom.AddTriangle(Vector3i(*i0, z), Vector3i(*i1, z), Vector3i(*i2, z));
			else
				geom.AddTriangle(Vector3i(*i0, z), Vector3i(*i2, z), Vector3i(*i1, z));

			vert.erase(i1);
		}

		if (++i0 == vert.end())
			++i0;

		if (--iters == 0) {
			/* FIXME: add way ID here, lacks interface to datasource */
			fprintf(stderr, "warning: triangulation failed: giving up on %d/%d points\n", vert.size(), vertices.size());
			return;
		}
	}
}

static void CreateRoof(Geometry& geom, const VertexList& vertices, int z, const OsmDatasource::Way& way) {
	OsmDatasource::TagsMap::const_iterator shape, tag;
	if (vertices.size() == 5 && way.Closed && (shape = way.Tags.find("building:roof:shape")) != way.Tags.end()) {
		std::vector<Vector3i> vert;
		vert.resize(5);

		float slope = 30.0;
		bool along = true;

		if ((tag = way.Tags.find("building:roof:angle")) != way.Tags.end())
			slope = strtof(tag->second.c_str(), NULL);
		if ((tag = way.Tags.find("building:roof:orientation")) != way.Tags.end() && tag->second == "across")
			along = false;

		/* tag 5 corner nodes, always clockwise (last node still duplicate of first for convenience) */
		if (way.Clockwise) {
			int j = 0;
			for (VertexList::const_iterator i = vertices.begin(); i != vertices.end(); ++i, ++j)
				vert[j] = Vector3i(*i, z);
		} else {
			int j = 4;
			for (VertexList::const_iterator i = vertices.begin(); i != vertices.end(); ++i, --j)
				vert[j] = Vector3i(*i, z);
		}

		float length1 = ToLocalMetric(vert[0], vert[1]).Length();
		float length2 = ToLocalMetric(vert[1], vert[2]).Length();

		if (shape->second == "pyramidal") {
			Vector3i center = ((Vector3l)vert[0] + (Vector3l)vert[1] + (Vector3l)vert[2] + (Vector3l)vert[3]) / 4;
			center.z += (tan(slope/180.0*M_PI) * std::min(length1, length2) * 0.5) * 1000.0;

			for (int i = 0; i < 4; i++) {
				geom.AddTriangle(vert[i], center, vert[i+1]);
				geom.AddLine(vert[i], center);
			}
			return;
		} else if (shape->second == "pitched") {
			if (!!(length1 < length2) ^ !along) {
				osmint_t height = (tan(slope/180.0*M_PI) * length1 * 0.5) * 1000.0;

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
				osmint_t height = (tan(slope/180.0*M_PI) * length2 * 0.5) * 1000.0;

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
				osmint_t height = (tan(slope/180.0*M_PI) * length1 * 0.5) * 1000.0;

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
				osmint_t height = (tan(slope/180.0*M_PI) * length2 * 0.5) * 1000.0;

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
			int height = (tan(slope/180.0*M_PI) * std::min(length1, length2) * 0.5) * 1000.0;

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

static void CreateRoad(Geometry& geom, const VertexList& vertices, float width, const OsmDatasource::Way& way) {
	if (vertices.size() < 2)
		return;

	VertexList::const_iterator prev = vertices.end();
	VertexList::const_iterator next;
	Vector2i prev_points[2];
	Vector2i new_points[2];
	for (VertexList::const_iterator i = vertices.begin(); i != vertices.end(); i++) {
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

static float GetMaxHeight(const OsmDatasource::Way& way) {
	OsmDatasource::TagsMap::const_iterator building, tag;

	float h = 0.0f; /* in meters */

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

static void WayDispatcher(Geometry& geom, const OsmDatasource& datasource, const OsmDatasource::Way& way) {
	osmint_t minz = GetMinHeight(way)*1000.0;
	osmint_t maxz = GetMaxHeight(way)*1000.0;

	if (minz < 0)
		minz = 0;
	if (maxz < minz) {
		fprintf(stderr, "warning: max height < min height for object\n");
		maxz = minz = 0;
	}

	OsmDatasource::TagsMap::const_iterator t;

	VertexList vertices;
	for (OsmDatasource::Way::NodesList::const_iterator n = way.Nodes.begin(); n != way.Nodes.end(); ++n)
		vertices.push_back(datasource.GetNode(*n).Pos);

	if ((way.Tags.find("building") != way.Tags.end() || way.Tags.find("building:part") != way.Tags.end()) && minz != maxz) {
		CreateWalls(geom, vertices, minz, maxz, way);
		CreateRoof(geom, vertices, maxz, way);

		CreateLines(geom, vertices, minz, way);
		CreateLines(geom, vertices, maxz, way);
		CreateSmartVerticalLines(geom, vertices, minz, maxz, 5.0, way);

		if (minz > 1) {
			CreateArea(geom, vertices, true, minz, way);
			CreateLines(geom, vertices, minz, way);
		}
	} else if (way.Tags.find("barrier") != way.Tags.end()) {
		if (maxz == minz)
			maxz += 2000;
		CreateWall(geom, vertices, minz, maxz, way);

		CreateLines(geom, vertices, minz, way);
		CreateLines(geom, vertices, maxz, way);
		CreateVerticalLines(geom, vertices, minz, maxz, way);
	} else if ((t = way.Tags.find("highway")) != way.Tags.end()) {
		OsmDatasource::TagsMap::const_iterator t1;

		if ((t1 = way.Tags.find("area")) != way.Tags.end() && t1->second != "no") {
			/* area */
			CreateArea(geom, vertices, false, 0, way);
		} else {
			CreateRoad(geom, vertices, GetHighwayWidth(t->second, way), way);
		}
	} else {
		CreateLines(geom, vertices, minz, way);
	}
}

DefaultGeometryGenerator::DefaultGeometryGenerator(const OsmDatasource& datasource) : datasource_(datasource) {
}

void DefaultGeometryGenerator::GetGeometry(Geometry& geom, const BBoxi& bbox) const {
	std::vector<OsmDatasource::Way> ways;

	/* safe bbox is a bit wider than requested one to be sure
	 * all ways are included, even those which have width */
	float extra_width = 24.0; /* still may be not sufficient, e.g. very wide roads */
	BBoxi safe_bbox = BBoxi(
			FromLocalMetric((Vector2d)ToLocalMetric(bbox.GetBottomLeft(), bbox.GetBottomLeft()) - extra_width, bbox.GetBottomLeft()),
			FromLocalMetric((Vector2d)ToLocalMetric(bbox.GetTopRight(), bbox.GetTopRight()) + extra_width, bbox.GetTopRight())
		);
	datasource_.GetWays(ways, safe_bbox);

	Geometry temp;
	for (std::vector<OsmDatasource::Way>::const_iterator w = ways.begin(); w != ways.end(); ++w) {
		WayDispatcher(temp, datasource_, *w);
	}

	geom.AppendCropped(temp, bbox);
}

Vector2i DefaultGeometryGenerator::GetCenter() const {
	return datasource_.GetCenter();
}

BBoxi DefaultGeometryGenerator::GetBBox() const {
	return datasource_.GetBBox();
}
