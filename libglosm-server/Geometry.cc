/*
 * Copyright (C) 2010-2012 Dmitry Marakasov
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

#include <glosm/Geometry.hh>

#include <glosm/GeometryOperations.hh>

#include <cassert>
#include <iostream>

Geometry::Geometry() {
}

void Geometry::AddLine(const Vector3i& a, const Vector3i& b) {
	lines_vertices_.push_back(a);
	lines_vertices_.push_back(b);
	lines_lengths_.push_back(2);
}

void Geometry::AddTriangle(const Vector3i& a, const Vector3i& b, const Vector3i& c) {
	convex_vertices_.push_back(a);
	convex_vertices_.push_back(b);
	convex_vertices_.push_back(c);
	convex_lengths_.push_back(3);
}

void Geometry::AddQuad(const Vector3i& a, const Vector3i& b, const Vector3i& c, const Vector3i& d) {
	convex_vertices_.push_back(a);
	convex_vertices_.push_back(b);
	convex_vertices_.push_back(c);
	convex_vertices_.push_back(d);
	convex_lengths_.push_back(4);
}

void Geometry::AddLine(const std::vector<Vector3i>& v) {
	lines_vertices_.insert(lines_vertices_.end(), v.begin(), v.end());
	lines_lengths_.push_back(v.size());
}

void Geometry::AddConvex(const std::vector<Vector3i>& v) {
	convex_vertices_.insert(convex_vertices_.end(), v.begin(), v.end());
	convex_lengths_.push_back(v.size());
}

void Geometry::StartLine() {
	lines_lengths_.push_back(0);
}

void Geometry::AppendLine(const Vector3i& v) {
	lines_vertices_.push_back(v);
	lines_lengths_.back()++;
}

void Geometry::StartConvex() {
	convex_lengths_.push_back(0);
}

void Geometry::AppendConvex(const Vector3i& v) {
	convex_vertices_.push_back(v);
	convex_lengths_.back()++;
}

const Geometry::VertexVector& Geometry::GetLinesVertices() const {
	return lines_vertices_;
}

const Geometry::LengthVector& Geometry::GetLinesLengths() const {
	return lines_lengths_;
}

const Geometry::VertexVector& Geometry::GetConvexVertices() const {
	return convex_vertices_;
}

const Geometry::LengthVector& Geometry::GetConvexLengths() const {
	return convex_lengths_;
}

void Geometry::Append(const Geometry& other) {
	convex_vertices_.reserve(convex_vertices_.size() + other.convex_vertices_.size());
	convex_vertices_.insert(convex_vertices_.end(), other.convex_vertices_.begin(), other.convex_vertices_.end());

	convex_lengths_.reserve(convex_lengths_.size() + other.convex_lengths_.size());
	convex_lengths_.insert(convex_lengths_.end(), other.convex_lengths_.begin(), other.convex_lengths_.end());

	lines_vertices_.reserve(lines_vertices_.size() + other.lines_vertices_.size());
	lines_vertices_.insert(lines_vertices_.end(), other.lines_vertices_.begin(), other.lines_vertices_.end());

	lines_lengths_.reserve(lines_lengths_.size() + other.lines_lengths_.size());
	lines_lengths_.insert(lines_lengths_.end(), other.lines_lengths_.begin(), other.lines_lengths_.end());
}

void Geometry::AppendCropped(const Geometry& other, const BBoxi& bbox) {
	for (unsigned int i = 0, curpos = 0; i < other.lines_lengths_.size(); ++i) {
		AddCroppedLine(&other.lines_vertices_[curpos], other.lines_lengths_[i], bbox);
		curpos += other.lines_lengths_[i];
	}

	for (unsigned int i = 0, curpos = 0; i < other.convex_lengths_.size(); ++i) {
		AddCroppedConvex(&other.convex_vertices_[curpos], other.convex_lengths_[i], bbox);
		curpos += other.convex_lengths_[i];
	}
}

void Geometry::AddCroppedConvex(const Vector3i* v, unsigned int size, const BBoxi& bbox) {
	struct VList {
		Vector3i vertex;
		VList* prev;
		VList* next;

		VList(): prev(NULL), next(NULL) {}
		VList(const Vector3i& v, VList* p, VList* n): vertex(v), prev(p), next(n) {}
	};

	/* construct circular linked list of vertices */
	bool all_vertices_in_bbox = true;

	/* ugly way of allocating a simple array; don't want to use
	 * new here to not care of freeing, and automatic array of
	 * VList is not allowed since VList is not POD */
	char buffer[sizeof(VList) * (size + 4)];
	VList* vertices = reinterpret_cast<VList*>(buffer);
	for (unsigned int i = 0; i < size; ++i) {
		vertices[i] = VList(v[i], &vertices[i-1], &vertices[i+1]);
		if (!bbox.Contains(v[i]))
			all_vertices_in_bbox = false;
	}

	/* don't run expensive algorithm if cropping is not required */
	if (all_vertices_in_bbox) {
		convex_vertices_.reserve(convex_vertices_.size() + size);
		for (unsigned int i = 0; i < size; ++i)
			convex_vertices_.push_back(v[i]);
		convex_lengths_.push_back(size);
		return;
	}

	unsigned int nvertices = size;
	vertices[0].prev = &vertices[nvertices - 1];
	vertices[nvertices - 1].next = &vertices[0];

	/* crop by each side of bbox */
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
						assert(nvertices < size + 4);
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

	int n = 1;
	convex_vertices_.push_back(p->vertex);
	for (first = p, p = p->next; p != first; p = p->next, ++n)
		convex_vertices_.push_back(p->vertex);
	convex_lengths_.push_back(n);
}

void Geometry::AddCroppedLine(const Vector3i* v, unsigned int size, const BBoxi& bbox) {
	bool contained_prev = false;
	for (unsigned int i = 0; i < size; ++i) {
		if (bbox.Contains(v[i])) {
			if (i == 0) {
				lines_lengths_.push_back(1);
			} else if (contained_prev) {
				lines_lengths_.back()++;
			} else {
				Vector3i intersection;
				IntersectSegmentWithBBox(v[i-1], v[i], bbox, intersection);
				lines_lengths_.push_back(2);
				lines_vertices_.push_back(intersection);
			}
			lines_vertices_.push_back(v[i]);
			contained_prev = true;
		} else {
			if (contained_prev) {
				Vector3i intersection;
				IntersectSegmentWithBBox(v[i-1], v[i], bbox, intersection);
				lines_vertices_.push_back(intersection);
				lines_lengths_.back()++;
			} else if (i != 0) {
				Vector3i intersection1, intersection2;
				if (CropSegmentByBBox(v[i-1], v[i], bbox, intersection1, intersection2)) {
					lines_vertices_.push_back(intersection1);
					lines_vertices_.push_back(intersection2);
					lines_lengths_.push_back(2);
				}
			}
			contained_prev = false;
		}
	}
}

void Geometry::Serialize() const {
	/* @todo implement serialization to stream/file/buffer/whatever */
}

void Geometry::DeSerialize() {
	/* @todo implement deserialization from stream/file/buffer/whatever */
}
