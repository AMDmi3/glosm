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
	HeightmapDatasource::Heightmap heightmap;

	datasource.GetHeightmap(bbox, 1, heightmap);

	int width = heightmap.width - 2;
	int height = heightmap.height - 2;

	/* for each tile, we store position and normal for each
	 * vertex in the grid; we also store index array which
	 * arranges vertices into a single triangle strip */

	vbo_.reset(new VertexBuffer<TerrainVertex>(GL_ARRAY_BUFFER));
	vbo_->Data().resize(width * height);

	/* project */
	std::vector<Vector3f> projected;
	projected.reserve(heightmap.width * heightmap.height);
	for (int y = 0; y < heightmap.height; ++y) {
		for (int x = 0; x < heightmap.width; ++x) {
			projected.push_back(projection.Project(Vector3i(
								(osmint_t)((double)heightmap.bbox.left) + ((double)heightmap.bbox.right - (double)heightmap.bbox.left) * ((double)x / (double)(heightmap.width - 1)),
								(osmint_t)((double)heightmap.bbox.bottom) + ((double)heightmap.bbox.top - (double)heightmap.bbox.bottom) * ((double)y / (double)(heightmap.height - 1)),
								heightmap.points[y * heightmap.width + x]
							), ref));
		}
	}

	/* prepare vertices & normals */
	int n = 0;
	for (int y = 1; y < heightmap.height - 1; ++y) {
		for (int x = 1; x < heightmap.width - 1; ++x) {
			Vector3f v1 = projected[y * heightmap.width + x + 1] - projected[y * heightmap.width + x - 1];
			Vector3f v2 = projected[(y + 1) * heightmap.width + x] - projected[(y - 1) * heightmap.width + x];

			vbo_->Data()[n].pos = projected[y * heightmap.width + x];
			vbo_->Data()[n].norm = v1.CrossProduct(v2).Normalized();

			n++;
		}
	}

	double k1, k2;
	double cellheight = ((double)heightmap.bbox.top - (double)heightmap.bbox.bottom) / (double)(heightmap.height - 1);
	double cellwidth = ((double)heightmap.bbox.right - (double)heightmap.bbox.left) / (double)(heightmap.width - 1);

	/*
	 * @todo this clamping is not quite correct: it doesn't
	 * take the fact that terrain grid consists of triangles
	 * into account
	 */

	/* clamp bottom & top */
	k1 = ((double)bbox.bottom - (double)heightmap.bbox.bottom) / cellheight - 1.0;
	k2 = ((double)heightmap.bbox.top - (double)bbox.top) / cellheight - 1.0;
	for (int x = 0; x < width; ++x) {
		vbo_->Data()[x].pos = vbo_->Data()[x].pos * (1.0 - k1) + vbo_->Data()[width + x].pos * (k1);
		vbo_->Data()[x].norm = vbo_->Data()[x].norm * (1.0 - k1) + vbo_->Data()[width + x].norm * (k1);

		vbo_->Data()[(height - 1) * width + x].pos = vbo_->Data()[(height - 1) * width + x].pos * (1.0 - k2) + vbo_->Data()[(height - 2) * width + x].pos * (k2);
		vbo_->Data()[(height - 1) * width + x].norm = vbo_->Data()[(height - 1) * width + x].norm * (1.0 - k2) + vbo_->Data()[(height - 2) * width + x].norm * (k2);
	}

	/* clamp left & right */
	k1 = ((double)bbox.left - (double)heightmap.bbox.left) / cellwidth - 1.0;
	k2 = ((double)heightmap.bbox.right - (double)bbox.right) / cellwidth - 1.0;
	for (int y = 0; y < height; ++y) {
		vbo_->Data()[y * width].pos = vbo_->Data()[y * width].pos * (1.0 - k1) + vbo_->Data()[y * width + 1].pos * (k1);
		vbo_->Data()[y * width].norm = vbo_->Data()[y * width].norm * (1.0 - k1) + vbo_->Data()[y * width + 1].norm * (k1);

		vbo_->Data()[y * width + width - 1].pos = vbo_->Data()[y * width + width - 1].pos * (1.0 - k2) + vbo_->Data()[y * width + width - 2].pos * (k2);
		vbo_->Data()[y * width + width - 1].norm = vbo_->Data()[y * width + width - 1].norm * (1.0 - k2) + vbo_->Data()[y * width + width - 2].norm * (k2);
	}

	if (vbo_->GetSize() > 65536)
		throw std::logic_error("error constructing TerrainTile: more than 65536 vertices were stored in a VBO which is indexed with SHORTs");

	/* prepare indices */
	ibo_.reset(new VertexBuffer<GLushort>(GL_ELEMENT_ARRAY_BUFFER));
	ibo_->Data().reserve((height - 1) * (width * 2 + 2) - 2);
	for (int y = 0; y < height - 1; ++y) {
		/* since strip is arranged per-row, we duplicate last and first
		 * vertices in each row to make triangle between rows degraded
		 * and thus not renderable */
		if (y > 0)
			ibo_->Data().push_back((y + 1) * width);
		for (int x = 0; x < width; ++x) {
			ibo_->Data().push_back((y + 1) * width + x);
			ibo_->Data().push_back(y * width + x);
		}
		if (y < height - 2)
			ibo_->Data().push_back(y * width + width - 1);
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

	//glPolygonMode(GL_FRONT, GL_LINE);
	glDrawElements(GL_TRIANGLE_STRIP, ibo_->GetSize(), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	//glPolygonMode(GL_FRONT, GL_FILL);

	ibo_->UnBind();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

size_t TerrainTile::GetSize() const {
	return size_;
}
