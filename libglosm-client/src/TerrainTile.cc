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
#include <glosm/SimpleVertexBuffer.hh>

TerrainTile::TerrainTile(const Projection& projection, HeightmapDatasource& datasource, const Vector2i& ref, const BBoxi& bbox) : Tile(ref) {
	std::vector<osmint_t> terrain;
	BBoxi real_bbox;
	Vector2<int> resolution;

	datasource.GetHeights(terrain, real_bbox, resolution, bbox);

	projected_triangles_.reset(new ProjectedVertices);

	for (int y = 0; y < resolution.y - 1; ++y) {
		for (int x = 0; x < resolution.x - 1; ++x) {
			Vector3i v[4];
			v[0] = Vector3i(
							(osmint_t)((double)real_bbox.left) + ((double)real_bbox.right - (double)real_bbox.left) * ((double)x / (double)(resolution.x - 1)),
							(osmint_t)((double)real_bbox.bottom) + ((double)real_bbox.top - (double)real_bbox.bottom) * ((double)y / (double)(resolution.y - 1)),
							terrain[y * resolution.x + x]
						);
			v[1] = Vector3i(
							(osmint_t)((double)real_bbox.left) + ((double)real_bbox.right - (double)real_bbox.left) * ((double)(x + 1) / (double)(resolution.x - 1)),
							(osmint_t)((double)real_bbox.bottom) + ((double)real_bbox.top - (double)real_bbox.bottom) * ((double)y / (double)(resolution.y - 1)),
							terrain[y * resolution.x + x + 1]
						);
			v[2] = Vector3i(
							(osmint_t)((double)real_bbox.left) + ((double)real_bbox.right - (double)real_bbox.left) * ((double)x / (double)(resolution.x - 1)),
							(osmint_t)((double)real_bbox.bottom) + ((double)real_bbox.top - (double)real_bbox.bottom) * ((double)(y + 1) / (double)(resolution.y - 1)),
							terrain[(y + 1) * resolution.x + x]
						);
			v[3] = Vector3i(
							(osmint_t)((double)real_bbox.left) + ((double)real_bbox.right - (double)real_bbox.left) * ((double)(x + 1) / (double)(resolution.x - 1)),
							(osmint_t)((double)real_bbox.bottom) + ((double)real_bbox.top - (double)real_bbox.bottom) * ((double)(y + 1) / (double)(resolution.y - 1)),
							terrain[(y + 1) * resolution.x + x + 1]
						);

			projected_triangles_->push_back(projection.Project(v[0], ref));
			projected_triangles_->push_back(projection.Project(v[3], ref));
			projected_triangles_->push_back(projection.Project(v[2], ref));

			projected_triangles_->push_back(projection.Project(v[0], ref));
			projected_triangles_->push_back(projection.Project(v[1], ref));
			projected_triangles_->push_back(projection.Project(v[3], ref));
		}
	}

	size_ = projected_triangles_->size() * 3 * sizeof(float);
}

TerrainTile::~TerrainTile() {
}

void TerrainTile::BindBuffers() {
	if (projected_triangles_.get()) {
		triangles_.reset(new SimpleVertexBuffer(SimpleVertexBuffer::TRIANGLES, projected_triangles_->data(), projected_triangles_->size()));
		projected_triangles_.reset(NULL);
	}
}

void TerrainTile::Render() {
	BindBuffers();

	if (triangles_.get())
		triangles_->Render();
}

size_t TerrainTile::GetSize() const {
	return size_;
}
