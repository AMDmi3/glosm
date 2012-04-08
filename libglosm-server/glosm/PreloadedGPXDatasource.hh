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

#ifndef PRELOADEDGPXDATASOURCE_HH
#define PRELOADEDGPXDATASOURCE_HH

#include <glosm/GPXDatasource.hh>
#include <glosm/XMLParser.hh>
#include <glosm/NonCopyable.hh>

#include <vector>

/**
 * Source of GPX data which preloads tracks into memory.
 *
 * @todo this needs quadtree or some other clever interface
 * to scale for large amounts of data.
 */
class PreloadedGPXDatasource : public XMLParser, public GPXDatasource, private NonCopyable {
protected:
	typedef std::vector<Vector3i> PointsVector;

protected:
	PointsVector points_;

	/* parser state */
	enum {
		NONE,
		GPX,
		TRK,
		TRKSEG,
		TRKPT,
		ELE,
	} current_tag_;

	int tag_level_;

protected:
	/**
	 * Processes start XML element
	 */
	virtual void StartElement(const char* name, const char** atts);

	/**
	 * Processes end XML element
	 */
	virtual void EndElement(const char* name);

	/**
	 * Processes end XML element
	 */
	virtual void CharacterData(const char* data, int len);

public:
	/**
	 * Constructs empty datasource
	 */
	PreloadedGPXDatasource();

	/**
	 * Destructor
	 */
	virtual ~PreloadedGPXDatasource();

	/**
	 * Parses OSM dump file and loads map data into memory
	 *
	 * @param filename path to dump file
	 */
	virtual void Load(const char* filename);

	virtual void GetPoints(std::vector<Vector3i>& out, const BBoxi& bbox) const;
};

#endif
