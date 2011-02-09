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

#ifndef PRELOADEDXMLDATASOURCE_HH
#define PRELOADEDXMLDATASOURCE_HH

#include <glosm/OsmDatasource.hh>
#include <glosm/Exception.hh>
#include <glosm/NonCopyable.hh>
#include <glosm/id_map.hh>

class ParsingException : public Exception {
};

class DataException : public Exception {
};

/**
 * Source of OpenStreetMap data which preloads .osm dump into memory.
 *
 * This is a simple OsmDatasource that first parses OSM XML dump
 * with XML parser and stores node/way/relation information in
 * memory.
 */
class PreloadedXmlDatasource : public OsmDatasource, private NonCopyable {
protected:
	typedef id_map<osmid_t, Node> NodesMap;
	//typedef id_map<osmid_t, TagsMap> NodeTagsMap;
	typedef id_map<osmid_t, Way> WaysMap;
	typedef id_map<osmid_t, Relation> RelationsMap;

protected:
	/* data */
	NodesMap nodes_;
	//NodeTagsMap node_tags_;
	WaysMap ways_;
	RelationsMap relations_;

	/* parser state */
	enum {
		NONE,
		NODE,
		WAY,
		RELATION,
	} InsideWhich;

	int tag_level_;

	NodesMap::iterator last_node_;
	//NodeTagsMap::iterator last_node_tags_;
	WaysMap::iterator last_way_;
	RelationsMap::iterator last_relation_;

	BBoxi bbox_;

protected:
	static void StartElement(void* userData, const char* name, const char** atts);
	static void EndElement(void* userData, const char* name);

public:
	PreloadedXmlDatasource();
	virtual ~PreloadedXmlDatasource();

	/**
	 * Loads OSM dump into memory
	 *
	 * @param filename path to dump file
	 */
	void Load(const char* filename);

	/**
	 * Drop all loaded data
	 *
	 * This is used to free memory used by loaded data when
	 * it's no longer needed. Datasource may be further reused.
	 */
	void Clear();

	virtual Vector2i GetCenter() const;
	virtual BBoxi GetBBox() const;

public:
	virtual const Node& GetNode(osmid_t id) const;
	virtual const Way& GetWay(osmid_t id) const;
	virtual const Relation& GetRelation(osmid_t id) const;

	virtual void GetWays(std::vector<Way>& out, const BBoxi& bbox) const;
};

#endif
