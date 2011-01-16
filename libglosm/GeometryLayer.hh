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

#include "Math.hh"
#include "Misc.hh"
#include "Layer.hh"
#include "Projection.hh"
#include "GeometryTile.hh"
#include "NonCopyable.hh"

#include <memory.h>

class Viewer;
class GeometryDatasource;

class GeometryLayer : public Layer, NonCopyable {
protected:
	const GeometryTile tile_;
	const Projection projection_;

public:
	GeometryLayer(const Projection projection, const GeometryDatasource& datasource);
	virtual ~GeometryLayer();
	virtual void RequestVisible(const BBoxi& bbox);
	virtual void Render(const Viewer& viewer) const;
};

#endif
