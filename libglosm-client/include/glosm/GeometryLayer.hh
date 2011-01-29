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

#ifndef GEOMETRYLAYER_HH
#define GEOMETRYLAYER_HH

#include <glosm/Math.hh>
#include <glosm/Misc.hh>
#include <glosm/Layer.hh>
#include <glosm/Projection.hh>
#include <glosm/GeometryTile.hh>
#include <glosm/NonCopyable.hh>
#include <glosm/TileManager.hh>

#include <memory.h>

class Viewer;
class GeometryDatasource;

/**
 * Layer with 3D OpenStreetMap data.
 */
class GeometryLayer : public Layer, public TileManager, NonCopyable {
protected:
	const Projection projection_;

public:
	GeometryLayer(const Projection projection, const GeometryDatasource& datasource);
	virtual ~GeometryLayer();

	void Render(const Viewer& viewer) const;
	virtual Tile* SpawnTile(const Geometry& geom) const;
};

#endif
