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

void Geometry::AppendCroppedTriangle(const Vector3i& a, const Vector3i& b, const Vector3i& c, const BBoxi& bbox) {
	int mask = 0;
	mask |= bbox.Contains(a) ? 1 : 0;
	mask |= bbox.Contains(b) ? 2 : 0;
	mask |= bbox.Contains(c) ? 4 : 0;

	switch (mask) {
	case 0: /* all out */
		break;
	case 1: /* a in, b out, c out */
		AppendCroppedTriangleIOO(a, b, c, bbox);
		break;
	case 2: /* a out, b in, c out */
		AppendCroppedTriangleIOO(b, c, a, bbox);
		break;
	case 3: /* a in, b in, c out */
		AppendCroppedTriangleIIO(a, b, c, bbox);
		break;
	case 4: /* a out, b out, c in */
		AppendCroppedTriangleIOO(c, a, b, bbox);
		break;
	case 5: /* a in, b out, c in */
		AppendCroppedTriangleIIO(c, a, b, bbox);
		break;
	case 6: /* a out, b in, c in */
		AppendCroppedTriangleIIO(b, c, a, bbox);
		break;
	case 7: /* all in */
		AddTriangle(a, b, c);
		break;
	}
}

void Geometry::AppendCroppedTriangleIIO(const Vector3i& a, const Vector3i& b, const Vector3i& c, const BBoxi& bbox) {
	Vector3i cb, ca;

	/* canculate expected intersections */
	if (!IntersectSegmentWithBBox(c, a, bbox, ca))
		assert(false);

	if (!IntersectSegmentWithBBox(c, b, bbox, cb))
		assert(false);

	if (ca.x == cb.x || ca.y == cb.y) {
		/* case 1: intersection points lie on a single bbox side */
		AddTriangle(a, b, cb);
		AddTriangle(a, cb, ca);
	} else {
		/* case 2: intersection points lie on different bbox sides */
	}
}

void Geometry::AppendCroppedTriangleIOO(const Vector3i& a, const Vector3i& b, const Vector3i& c, const BBoxi& bbox) {
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
		AppendCroppedTriangle(other.triangles_[i], other.triangles_[i+1], other.triangles_[i+2], bbox);

	for (size_t i = 0; i < other.quads_.size(); i += 4) {
		if (bbox.Contains(other.quads_[i]) && bbox.Contains(other.quads_[i+1]) && bbox.Contains(other.quads_[i+2]) && bbox.Contains(other.quads_[i+3])) {
			AddQuad(other.quads_[i], other.quads_[i+1], other.quads_[i+2], other.quads_[i+3]);
		} else {
			AppendCroppedTriangle(other.quads_[i], other.quads_[i+1], other.quads_[i+2], bbox);
			AppendCroppedTriangle(other.quads_[i+2], other.quads_[i+3], other.quads_[i], bbox);
		}
	}
}

void Geometry::Serialize() const {
	/* XXX: Implement serialization to stream/file/buffer/whatever */
}

void Geometry::DeSerialize() {
	/* XXX: Implement deserialization from stream/file/buffer/whatever */
}
