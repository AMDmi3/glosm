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

#include <glosm/Geometry.hh>
#include <glosm/GeometryOperations.hh>

#include <cassert>

Geometry::Geometry(): bbox_(BBoxi::Full()) {
}

Geometry::Geometry(const BBoxi& bbox): bbox_(bbox) {
}

const BBoxi& Geometry::GetBBox() const {
	return bbox_;
}

void Geometry::AddLine(const Vector3i& a, const Vector3i& b) {
	lines_.push_back(a);
	lines_.push_back(b);
}

void Geometry::AddTriangle(const Vector3i& a, const Vector3i& b, const Vector3i& c) {
	triangles_.push_back(a);
	triangles_.push_back(b);
	triangles_.push_back(c);
}

void Geometry::AddQuad(const Vector3i& a, const Vector3i& b, const Vector3i& c, const Vector3i& d) {
	quads_.push_back(a);
	quads_.push_back(b);
	quads_.push_back(c);
	quads_.push_back(d);
}

const std::vector<Vector3i>& Geometry::GetLines() const {
	return lines_;
}

const std::vector<Vector3i>& Geometry::GetTriangles() const {
	return triangles_;
}

const std::vector<Vector3i>& Geometry::GetQuads() const {
	return quads_;
}

void Geometry::Append(const Geometry& other) {
	lines_.reserve(lines_.size() + other.lines_.size());
	triangles_.reserve(triangles_.size() + other.triangles_.size());
	quads_.reserve(quads_.size() + other.quads_.size());

	lines_.insert(lines_.end(), other.lines_.begin(), other.lines_.end());
	triangles_.insert(triangles_.end(), other.triangles_.begin(), other.triangles_.end());
	quads_.insert(quads_.end(), other.quads_.begin(), other.quads_.end());
}

void Geometry::AppendCropped(const Geometry& other, const BBoxi& bbox) {
	lines_.reserve(lines_.size() + other.lines_.size());
	triangles_.reserve(triangles_.size() + other.triangles_.size());
	quads_.reserve(quads_.size() + other.quads_.size());

	Vector3i a, b, c;
	for (size_t i = 0; i < other.lines_.size(); i += 2) {
		if (bbox.Contains(other.lines_[i]) && bbox.Contains(other.lines_[i+1]))
			AddLine(other.lines_[i], other.lines_[i+1]);
		else if (CropSegmentByBBox(other.lines_[i], other.lines_[i+1], bbox, a, b))
			AddLine(a, b);
	}

	for (size_t i = 0; i < other.triangles_.size(); i += 3)
		AddCroppedTriangle(other.triangles_[i], other.triangles_[i+1], other.triangles_[i+2], bbox);

	for (size_t i = 0; i < other.quads_.size(); i += 4) {
		if (bbox.Contains(other.quads_[i]) && bbox.Contains(other.quads_[i+1]) && bbox.Contains(other.quads_[i+2]) && bbox.Contains(other.quads_[i+3])) {
			AddQuad(other.quads_[i], other.quads_[i+1], other.quads_[i+2], other.quads_[i+3]);
		} else {
			AddCroppedTriangle(other.quads_[i], other.quads_[i+1], other.quads_[i+2], bbox);
			AddCroppedTriangle(other.quads_[i+2], other.quads_[i+3], other.quads_[i], bbox);
		}
	}
}

void Geometry::AddCroppedTriangle(const Vector3i& a, const Vector3i& b, const Vector3i& c, const BBoxi& bbox) {
	struct VList {
		Vector3i vertex;
		VList* prev;
		VList* next;
		bool last;

		VList() {}
		VList(const Vector3i& v, VList* p, VList* n): vertex(v), prev(p), next(n), last(false) {}
	};

	VList vertices[6];
	vertices[0] = VList(a, &vertices[2], &vertices[1]);
	vertices[1] = VList(b, &vertices[0], &vertices[2]);
	vertices[2] = VList(c, &vertices[1], &vertices[0]);
	int nvertices = 3;

	/* crop left */
	Vector3i prevint, nextint;
	VList* p = &vertices[0];
	VList* first;
	for (int i = 1; i <= 4; ++i) {
		first = NULL; /* will be set to the first non-outbound vertex */
		do {
			/* check all vertices and crop them by bound */
			if (bbox.IsPointOutAtSide(p->vertex, (BBoxi::Side)i)) {
				if (IntersectSegmentWithBBoxSideNI(p->vertex, p->prev->vertex, bbox, (BBoxi::Side)i, prevint)) {
					if (IntersectSegmentWithBBoxSideNI(p->vertex, p->next->vertex, bbox, (BBoxi::Side)i, nextint)) {
						/* both edges have intersection -> add extra vertex (trapezoid case) */
						assert(nvertices < 6);
						vertices[nvertices] = VList(prevint, p->prev, p);
						p->vertex = nextint;
						p->prev->next = &vertices[nvertices];
						p->prev = &vertices[nvertices];
						nvertices++;
						if (!first)
							first = p->prev;
					} else {
						/* only prev edge has intersection - just move vertex */
						p->vertex = prevint;
						if (!first)
							first = p;
					}
				} else {
					if (IntersectSegmentWithBBoxSideNI(p->vertex, p->next->vertex, bbox, (BBoxi::Side)i, nextint)) {
						/* only next edge has intersection - just move vertex */
						p->vertex = nextint;
						if (!first)
							first = p;
					} else {
						/* no intersections - vertex may just be dropped */
						if (p->prev == p->next)
							return; /* triangle is completely outside of bbox */

						p->prev->next = p->next;
						p->next->prev = p->prev;
						p = p->prev;
					}
				}
			} else if (!first)
				first = p;
		} while ((p = p->next) != first);
	}

	first = p;
	p = p->next->next;
	do {
		AddTriangle(first->vertex, p->prev->vertex, p->vertex);
		p = p->next;
	} while (p != first);
}

void Geometry::Serialize() const {
	/* XXX: Implement serialization to stream/file/buffer/whatever */
}

void Geometry::DeSerialize() {
	/* XXX: Implement deserialization from stream/file/buffer/whatever */
}
