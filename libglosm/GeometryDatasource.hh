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

#ifndef GEOMETRYDATASOURCE_HH
#define GEOMETRYDATASOURCE_HH

#include "GeometryGenerator.hh"

class Geometry;
class OsmDatasource;

/* Currently is just a class, but this should really be an interface;
 * possible implementation is caching geometry datasource and/or
 * datasource that downloads from the web */

/**
 * A source of geometry used by GeometryLayer
 */
class GeometryDatasource {
protected:
	const OsmDatasource& osm_datasource_;

public:
	GeometryDatasource(const OsmDatasource& datasource);

	/* TODO: make it possible to change geom generator from outside, like:
	auto_ptr<Generator> generator_;
	template <class G>
	void SetGenerator() {
		generator_.Reset(new G(datasource_));
	}
	 */

	virtual void GetGeometry(Geometry& geometry, const BBoxi& bbox = BBoxi::Full()) const;
	virtual Vector2i GetCenter() const;
	virtual BBoxi GetBBox() const;
};

#endif
