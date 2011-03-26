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

#include <glosm/util/gl.h>

#include <glosm/TerrainTile.hh>
#include <glosm/HeightmapDatasource.hh>

#include <glosm/Projection.hh>
#include <glosm/VertexBuffer.hh>

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

	vbo_.reset(new VertexBuffer<TerrainVertex>(GL_ARRAY_BUFFER));
	vbo_->Data().resize(resolution.x * resolution.y);

	/* prepare vertices */
	int n = 0;
	for (int y = 0; y < resolution.y; ++y) {
		for (int x = 0; x < resolution.x; ++x) {
			vbo_->Data()[n++].pos = projection.Project(Vector3i(
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
				Vector3f v1 = vbo_->Data()[y * resolution.x + x + 1].pos - vbo_->Data()[y * resolution.x + x - 1].pos;
				Vector3f v2 = vbo_->Data()[(y + 1) * resolution.x + x].pos - vbo_->Data()[(y - 1) * resolution.x + x].pos;
				vbo_->Data()[n++].norm = v1.CrossProduct(v2).Normalized();
			} else {
				vbo_->Data()[n++].norm = Vector3f(0.0f, 0.0f, 1.0f);
			}
		}
	}

	if (vbo_->GetSize() > 65536)
		throw std::logic_error("error constructing TerrainTile: more than 65536 vertices were stored in a VBO which is indexed with SHORTs");

	/* prepare indices */
	ibo_.reset(new VertexBuffer<GLushort>(GL_ELEMENT_ARRAY_BUFFER));
	ibo_->Data().reserve((resolution.y - 1) * (resolution.x * 2 + 2) - 2);
	for (int y = 0; y < resolution.y - 1; ++y) {
		/* since strip is arranged per-row, we duplicate last and first
		 * vertices in each row to make triangle between rows degraded
		 * and thus not renderable */
		if (y > 0)
			ibo_->Data().push_back((y + 1) * resolution.x);
		for (int x = 0; x < resolution.x; ++x) {
			ibo_->Data().push_back((y + 1) * resolution.x + x);
			ibo_->Data().push_back(y * resolution.x + x);
		}
		if (y < resolution.y - 2)
			ibo_->Data().push_back(y * resolution.x + resolution.x - 1);
	}

	size_ = vbo_->GetFootprint() + ibo_->GetFootprint();
}

TerrainTile::~TerrainTile() {
}

void TerrainTile::Render() {
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
