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

#include <glosm/util/gl.h>

#include <glosm/GPXTile.hh>

#include <glosm/Projection.hh>
#include <glosm/SimpleVertexBuffer.hh>

GPXTile::GPXTile(const Projection& projection, const std::vector<Vector3i>& points, const Vector2i& ref, const BBoxi& bbox) : Tile(ref) {
	size_ = points.size() * sizeof(float);

	if (points.size() != 0) {
		projected_points_.reset(new ProjectedVertices);
		projection.ProjectPoints(points, ref, *projected_points_);
	}
}

GPXTile::~GPXTile() {
}

void GPXTile::BindBuffers() {
	if (projected_points_.get()) {
		points_.reset(new SimpleVertexBuffer(SimpleVertexBuffer::POINTS, projected_points_->data(), projected_points_->size()));
		projected_points_.reset(NULL);
	}
}

void GPXTile::Render() {
	BindBuffers();

	if (points_.get()) {
		glDepthFunc(GL_LEQUAL);

		glColor4f(1.0f, 0.0f, 1.0f, 0.5f);
		glPointSize(3.0);
		points_->Render();
	}
}

size_t GPXTile::GetSize() const {
	return size_;
}
