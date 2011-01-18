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

void Geometry::Serialize() const {
	/* XXX: Implement serialization to stream/file/buffer/whatever */
}

void Geometry::DeSerialize() {
	/* XXX: Implement deserialization from stream/file/buffer/whatever */
}
