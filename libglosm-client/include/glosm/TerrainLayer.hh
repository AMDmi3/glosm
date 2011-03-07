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

#ifndef TERRAINLAYER_HH
#define TERRAINLAYER_HH

#include <glosm/Layer.hh>
#include <glosm/Projection.hh>
#include <glosm/NonCopyable.hh>
#include <glosm/TileManager.hh>

class Viewer;
class HeightmapDatasource;

/**
 * Layer with 3D terrain data.
 */
class TerrainLayer : public Layer, public TileManager, private NonCopyable {
protected:
	const Projection projection_;
	HeightmapDatasource& datasource_;

public:
	TerrainLayer(const Projection projection, HeightmapDatasource& datasource);
	virtual ~TerrainLayer();

	void Render(const Viewer& viewer);
	virtual Tile* SpawnTile(const BBoxi& bbox, int flags) const;
};

#endif
