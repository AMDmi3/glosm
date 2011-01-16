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

/*
 * Generic class for loading and storing OSM data
 *
 * Possible generic improvements:
 * - use hash maps instead of tree maps
 * - store tag names separately and operate with tag id
 *   - use (hash)map for forward lookup (tag -> id)
 *   - use vector for reverse lookup (id -> tag)
 * - use fastosm library
 * - use custom allocators for most data
 *
 * Space iprovements with complexity/speed cost
 * - prefix encoding for node coords and refs
 * - store nodes without tags in a separate or additional map
 * - store node coords directly in ways
 * Last two should save tons of memory and improve speed severely
 * by killing an indirection
 *
 * Improvements for non-generic use:
 * (tons)
 *
 * Other:
 * - may store relation id(s) for ways - at least for multipolygons
 */

#include "PreloadedXmlDatasource.hh"

#include <stdexcept>
#include <limits>

#include <fcntl.h>

#include <expat.h>
#include <string.h>

/* when we know that a string may be only one of e.g. "node", "way",
 * "relation", we can only check first letter which will give us
 * ~10% parsing performance, however this increases chance
 *
 * this is shortcut for string comparison used where we know
 * that the string e.g. bay only be 'way', 'node', or 'relation'
 * in which case we can only check first letter - this lends
 * ~10% performance gain */
#ifdef TRUSTED_XML
template <int I>
inline bool StrEq(const char* one, const char* two) {
	return strncmp(one, two, I) == 0;
}

template<>
inline bool StrEq<-1>(const char* one, const char* two) {
	return strcmp(one, two) == 0;
}

template<>
inline bool StrEq<0>(const char* one, const char* two) {
	return true;
}

template<>
inline bool StrEq<1>(const char* one, const char* two) {
	return one[0] == two[0];
}

template<>
inline bool StrEq<2>(const char* one, const char* two) {
	return one[0] == two[0] && one[0] != '\0' && one[1] == two[1];
}
#else
template <int I>
inline bool StrEq(const char* one, const char* two) {
	return strcmp(one, two) == 0;
}
#endif

/* Parse langitude/latitude in osm format, e.g. [-]NNN.NNNNNNN */
static int ParseCoord(const char* str) {
	int value = 0;
	int fracdig = 0;
	int haddot = 0;
	bool neg = false;
	const char* cur = str;

	if (*cur == '-') {
		neg = true;
		cur++;
	}

	for (; *cur != '\0'; ++cur) {
		if (*cur >= '0' && *cur <= '9') {
			value = value * 10 + *cur - '0';
			if (haddot && ++fracdig == 7)
				break;
		} else if (*cur == '.') {
			haddot++;
		} else {
			throw std::runtime_error("bad coordinate format (unexpected symbol)");
		}
	}

	if (haddot > 1)
		throw std::runtime_error("bad coordinate format (multiple dots)");

	if (fracdig > 7)
		throw std::runtime_error("bad coordinate format (too long)");

	for (; fracdig < 7; ++fracdig)
		value *= 10;

	return neg ? -value : value;
}

static void ParseTag(OsmDatasource::TagsMap& map, const char** atts) {
	std::string key;
	std::string value;
	for (const char** att = atts; *att; ++att) {
		if (StrEq<1>(*att, "k"))
			key = *(++att);
		else if (StrEq<1>(*att, "v"))
			value = *(++att);
		else
			++att;
	}

	map.insert(std::make_pair(key, value));
}

static BBoxi ParseBounds(const char** atts) {
	BBoxi bbox(BBoxi::Empty());

	for (const char** att = atts; *att; ++att) {
		if (StrEq<-1>(*att, "minlat"))
			bbox.bottom = ParseCoord(*(++att));
		else if (StrEq<-1>(*att, "maxlat"))
			bbox.top = ParseCoord(*(++att));
		else if (StrEq<-1>(*att, "minlon"))
			bbox.left = ParseCoord(*(++att));
		else if (StrEq<-1>(*att, "maxlon"))
			bbox.right = ParseCoord(*(++att));
		else
			++att;
	}

	return bbox;
}

static BBoxi ParseBound(const char** atts) {
	BBoxi bbox(BBoxi::Empty());

	for (const char** att = atts; *att; ++att) {
		if (StrEq<-1>(*att, "box")) {
			std::string s(*(++att));
			/* comma positions */
			size_t cpos1, cpos2, cpos3;
			if ((cpos1 = s.find(',')) == std::string::npos)
				throw std::runtime_error("bad bbox format");
			if ((cpos2 = s.find(',', cpos1+1)) == std::string::npos)
				throw std::runtime_error("bad bbox format");
			if ((cpos3 = s.find(',', cpos2+1)) == std::string::npos)
				throw std::runtime_error("bad bbox format");

			bbox.bottom = ParseCoord(s.substr(0, cpos1).c_str());
			bbox.left = ParseCoord(s.substr(cpos1+1, cpos2-cpos1-1).c_str());
			bbox.top = ParseCoord(s.substr(cpos2+1, cpos3-cpos2-1).c_str());
			bbox.right = ParseCoord(s.substr(cpos3+1).c_str());
		} else {
			++att;
		}
	}

	return bbox;
}

PreloadedXmlDatasource::PreloadedXmlDatasource() : bbox_(BBoxi::Empty()) {
}

PreloadedXmlDatasource::~PreloadedXmlDatasource() {
}

void PreloadedXmlDatasource::StartElement(void* userData, const char* name, const char** atts) {
	PreloadedXmlDatasource* loader = static_cast<PreloadedXmlDatasource*>(userData);

	if (loader->tag_level_ == 1 && loader->InsideWhich == NONE) {
		int id = 0;
		int lat = 0;
		int lon = 0;
		for (const char** att = atts; *att; ++att) {
			if (StrEq<1>(*att, "id"))
				id = strtol(*(++att), NULL, 10);
			else if (StrEq<2>(*att, "lat"))
				lat = ParseCoord(*(++att));
			else if (StrEq<2>(*att, "lon"))
				lon = ParseCoord(*(++att));
			else
				++att;
		}

		if (StrEq<1>(name, "node")) {
			loader->InsideWhich = NODE;
			std::pair<NodesMap::iterator, bool> p = loader->nodes_.insert(std::make_pair(id, Node(lon, lat)));
			loader->last_node_ = p.first;
			//loader->last_node_tags_ = loader->node_tags_.end();
		} else if (StrEq<1>(name, "way")) {
			loader->InsideWhich = WAY;
			std::pair<WaysMap::iterator, bool> p = loader->ways_.insert(std::make_pair(id, Way()));
			loader->last_way_ = p.first;
		} else if (StrEq<1>(name, "relation")) {
			loader->InsideWhich = RELATION;
			std::pair<RelationsMap::iterator, bool> p = loader->relations_.insert(std::make_pair(id, Relation()));
			loader->last_relation_ = p.first;
		} else if (StrEq<-1>(name, "bounds")) {
			loader->bbox_ = ParseBounds(atts);
		} else if (StrEq<-1>(name, "bound")) {
			loader->bbox_ = ParseBound(atts);
		}
	} else if (loader->tag_level_ == 2 && loader->InsideWhich == NODE) {
		if (StrEq<0>(name, "tag")) {
//			if (loader->last_node_tags_ == loader->node_tags_.end()) {
//				std::pair<NodeTagsMap::iterator, bool> p = loader->node_tags_.insert(std::make_pair(loader->last_node_->first, TagsMap()));
//				loader->last_node_tags_ = p.first;
//			}
//			ParseTag(loader->last_node_tags_->second, atts);
		} else {
			throw std::runtime_error("unexpected tag in node");
		}
	} else if (loader->tag_level_ == 2 && loader->InsideWhich == WAY) {
		if (StrEq<1>(name, "tag")) {
			ParseTag(loader->last_way_->second.Tags, atts);
		} else if (StrEq<1>(name, "nd")) {
			int id;

			if (**atts && StrEq<0>(*atts, "ref"))
				id = strtol(*(atts+1), NULL, 10);
			else
				throw std::runtime_error("no ref attribute for nd tag");

			loader->last_way_->second.Nodes.push_back(id);
		} else {
			throw std::runtime_error("unexpected tag in way");
		}
	} else if (loader->tag_level_ == 2 && loader->InsideWhich == RELATION) {
		if (StrEq<1>(name, "tag")) {
//			ParseTag(loader->last_relation_->second.Tags, atts);
		} else if (StrEq<1>(name, "member")) {
			int ref;
			const char* role;
			Relation::Member::Type_t type;

			for (const char** att = atts; *att; ++att) {
				if (StrEq<2>(*att, "ref"))
					ref = strtol(*(++att), NULL, 10);
				else if (StrEq<1>(*att, "type")) {
					++att;
					if (StrEq<1>(*att, "node"))
						type = Relation::Member::NODE;
					else if (StrEq<1>(*att, "way"))
						type = Relation::Member::WAY;
					else if (StrEq<1>(*att, "relation"))
						type = Relation::Member::RELATION;
					else
						throw std::runtime_error("bad relation member role");
				} else if (StrEq<2>(*att, "role")) {
					role = *(++att);
				} else {
					throw std::runtime_error("unexpected attribute in relation member");
				}
			}

			loader->last_relation_->second.Members.push_back(Relation::Member(type, ref, role));
		} else {
			throw std::runtime_error("unexpected tag in relation");
		}
	} else if (loader->tag_level_ >= 2) {
		throw std::runtime_error("unexpected tag");
	}

	++loader->tag_level_;
}

void PreloadedXmlDatasource::EndElement(void* userData, const char* /*name*/) {
	PreloadedXmlDatasource* loader = static_cast<PreloadedXmlDatasource*>(userData);

	if (loader->tag_level_ == 2) {
		switch (loader->InsideWhich) {
		case WAY:
			/* check if a way is closed */
			if (loader->last_way_->second.Nodes.front() == loader->last_way_->second.Nodes.back()) {
				loader->last_way_->second.Closed = true;

				/* check if a way is clockwise */
				NodesMap::const_iterator prev, cur;
				osmlong_t area = 0;
				for (Way::NodesList::const_iterator i = loader->last_way_->second.Nodes.begin(); i != loader->last_way_->second.Nodes.end(); ++i) {
					cur = loader->nodes_.find(*i);
					if (cur == loader->nodes_.end())
						throw std::runtime_error("referenced node not found");
					if (i != loader->last_way_->second.Nodes.begin())
						area += (osmlong_t)prev->second.Pos.x * cur->second.Pos.y - (osmlong_t)cur->second.Pos.x * prev->second.Pos.y;
					prev = cur;
				}

				loader->last_way_->second.Clockwise = area < 0;
			}
			break;
		default:
			break;
		}
		loader->InsideWhich = NONE;
	}

	--loader->tag_level_;
}

Vector2i PreloadedXmlDatasource::GetCenter() const {
	return bbox_.GetCenter();
}

BBoxi PreloadedXmlDatasource::GetBBox() const {
	return bbox_;
}

void PreloadedXmlDatasource::Load(const char* filename) {
	int f = -1;
	XML_Parser parser = NULL;

	bbox_ = BBoxi::Empty();

	try {
		InsideWhich = NONE;
		tag_level_ = 0;

		if ((f = open(filename, O_RDONLY)) == -1)
			throw std::runtime_error("cannot open XML file");

		/* Create and setup parser */
		if ((parser = XML_ParserCreate(NULL)) == NULL)
			throw std::runtime_error("cannot create XML parser");

		XML_SetElementHandler(parser, StartElement, EndElement);
		XML_SetUserData(parser, this);

		/* Parse file */
		char buf[65536];
		size_t len;
		do {
			len = read(f, buf, sizeof(buf));
			if (XML_Parse(parser, buf, len, len == 0) == XML_STATUS_ERROR)
				throw std::runtime_error("parsing error");
		} while (len != 0);
	} catch(...) {
		if (f != -1)
			close(f);
		if (parser != NULL)
			XML_ParserFree(parser);
		throw;
	}

	/* Cleanup */
	XML_ParserFree(parser);
	close(f);

	/* Postprocessing */
	if (bbox_.IsEmpty()) {
		/* bounding box in the file missing or is invalid - generate ourselves */
		for (NodesMap::iterator node = nodes_.begin(); node != nodes_.end(); ++node)
			bbox_.Include(node->second.Pos);
	}
}

void PreloadedXmlDatasource::Clear() {
	nodes_.clear();
	ways_.clear();
	relations_.clear();
}

const OsmDatasource::Node& PreloadedXmlDatasource::GetNode(osmid_t id) const {
	NodesMap::const_iterator i = nodes_.find(id);
	if (i == nodes_.end())
		throw std::runtime_error("node not found");
	return i->second;
}

const OsmDatasource::Way& PreloadedXmlDatasource::GetWay(osmid_t id) const {
	WaysMap::const_iterator i = ways_.find(id);
	if (i == ways_.end())
		throw std::runtime_error("way not found");
	return i->second;
}

const OsmDatasource::Relation& PreloadedXmlDatasource::GetRelation(osmid_t id) const {
	RelationsMap::const_iterator i = relations_.find(id);
	if (i == relations_.end())
		throw std::runtime_error("relation not found");
	return i->second;
}

void PreloadedXmlDatasource::GetAllWays(std::vector<OsmDatasource::Way>& out) const {
	for (WaysMap::const_iterator i = ways_.begin(); i != ways_.end(); ++i)
		out.push_back(i->second);
}
