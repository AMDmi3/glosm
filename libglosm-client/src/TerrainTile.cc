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

#include <glosm/TerrainTile.hh>
#include <glosm/HeightmapDatasource.hh>

#include <glosm/Projection.hh>
#include <glosm/VBO.hh>

#include <cassert>
#include <stdexcept>

TerrainTile::TerrainTile(const Projection& projection, HeightmapDatasource& datasource, const Vector2i& ref, const BBoxi& bbox) : Tile(ref) {
	std::vector<osmint_t> terrain;
	BBoxi real_bbox;
	Vector2<int> resolution;

	datasource.GetHeights(terrain, real_bbox, resolution, bbox);

	/* for each tile, we store position and normal for each
	 * vertex in the grid; we also store index array which
	 * arranges vertices into a single triangle strip */

	vertices_.reset(new VertexVector);
	vertices_->resize(resolution.x * resolution.y);

	/* prepare vertices */
	int n = 0;
	for (int y = 0; y < resolution.y; ++y) {
		for (int x = 0; x < resolution.x; ++x) {
			(*vertices_)[n++].pos = projection.Project(Vector3i(
								(osmint_t)((double)real_bbox.left) + ((double)real_bbox.right - (double)real_bbox.left) * ((double)x / (double)(resolution.x - 1)),
								(osmint_t)((double)real_bbox.bottom) + ((double)real_bbox.top - (double)real_bbox.bottom) * ((double)y / (double)(resolution.y - 1)),
								terrain[y * resolution.x + x]
							), ref);
		}
	}

	/* prepare normals */
	n = 0;
	for (int y = 0; y < resolution.y; ++y) {
		for (int x = 0; x < resolution.x; ++x) {
			if (x > 0 && x < resolution.x - 1 && y > 0 && y < resolution.y - 1) {
				Vector3f v1 = (*vertices_)[y * resolution.x + x + 1].pos - (*vertices_)[y * resolution.x + x - 1].pos;
				Vector3f v2 = (*vertices_)[(y + 1) * resolution.x + x].pos - (*vertices_)[(y - 1) * resolution.x + x].pos;
				(*vertices_)[n++].norm = v1.CrossProduct(v2).Normalized();
			} else {
				(*vertices_)[n++].norm = Vector3f(0.0f, 0.0f, 1.0f);
			}
		}
	}

	if (vertices_->size() > 65536)
		throw std::logic_error("error constructing TerrainTile: more than 65536 vertices were stored in a VBO which is indexed with SHORTs");

	/* prepare indices */
	indices_.reset(new IndexVector);
	indices_->reserve((resolution.y - 1) * (resolution.x * 2 + 2) - 2);
	for (int y = 0; y < resolution.y - 1; ++y) {
		/* since strip is arranged per-row, we duplicate last and first
		 * vertices in each row to make triangle between rows degraded
		 * and thus not renderable */
		if (y > 0)
			indices_->push_back((y + 1) * resolution.x);
		for (int x = 0; x < resolution.x; ++x) {
			indices_->push_back((y + 1) * resolution.x + x);
			indices_->push_back(y * resolution.x + x);
		}
		if (y < resolution.y - 2)
			indices_->push_back(y * resolution.x + resolution.x - 1);
	}

	size_ = vertices_->size() * sizeof(TerrainVertex) + indices_->size() * sizeof(GLushort);
}

TerrainTile::~TerrainTile() {
}

void TerrainTile::BindBuffers() {
	if (vertices_.get()) {
		vbo_.reset(new VBO<TerrainVertex>(vertices_->data(), vertices_->size()));
		vertices_.reset(NULL);
	}

	if (indices_.get()) {
		ibo_.reset(new IBO(indices_->data(), indices_->size()));
		indices_.reset(NULL);
	}
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void TerrainTile::Render() {
	BindBuffers();

	vbo_->Bind();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(TerrainVertex), BUFFER_OFFSET(0));

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(TerrainVertex), BUFFER_OFFSET(12));

	ibo_->Bind();

	glDrawElements(GL_TRIANGLE_STRIP, ibo_->GetSize(), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	ibo_->UnBind();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

size_t TerrainTile::GetSize() const {
	return size_;
}
