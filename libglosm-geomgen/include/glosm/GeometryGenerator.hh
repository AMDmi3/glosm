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

#ifndef GEOMETRYGENERATOR_HH
#define GEOMETRYGENERATOR_HH

#include <glosm/GeometryDatasource.hh>
#include <glosm/Math.hh>
#include <glosm/BBox.hh>

class OsmDatasource;
class Geometry;

class GeometryGenerator : public GeometryDatasource {
protected:
	const OsmDatasource& datasource_;

public:
	GeometryGenerator(const OsmDatasource& datasource);
	void GetGeometry(Geometry& geometry, const BBoxi& bbox = BBoxi::ForEarth()) const;

	virtual Vector2i GetCenter() const;
	virtual BBoxi GetBBox() const;
};

#endif
