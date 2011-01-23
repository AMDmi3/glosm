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

#ifndef OSMDATASOURCE_HH
#define OSMDATASOURCE_HH

#include <glosm/Math.hh>
#include <glosm/BBox.hh>

#include <string>
#include <vector>
#include <map>

/**
 * Abstract base class for sources of OpenStreetMap data.
 *
 * Provides a way for other components to request map data in the
 * form of nodes, ways and relations.
 */
class OsmDatasource {
public:
	typedef std::map<std::string, std::string> TagsMap;

public:
	struct Node {
		Vector2i Pos;

		Node() {}
		Node(int x, int y) : Pos(x, y) {}
	};

	struct Way {
		typedef std::vector<osmid_t> NodesList;

		NodesList Nodes;

		TagsMap Tags;
		bool Closed;
		bool Clockwise;

		BBoxi BBox;

		Way() : Closed(false), Clockwise(false), BBox(BBoxi::Empty()) {
		}
	};

	struct Relation {
		struct Member {
			enum Type_t {
				WAY,
				NODE,
				RELATION,
			} Type;

			osmid_t Ref;
			std::string Role;

			Member(Type_t type, osmid_t ref, const char* role): Type(type), Ref(ref), Role(role) {}
		};

		typedef std::vector<Member> MemberList;

		MemberList Members;
		//TagsMap Tags;
	};

public:
	/** Returns node by its id */
	virtual const Node& GetNode(osmid_t id) const = 0;

	/** Returns way by its id */
	virtual const Way& GetWay(osmid_t id) const = 0;

	/** Returns relation by its id */
	virtual const Relation& GetRelation(osmid_t id) const = 0;

	/* multiple - object accessors subject to change */
	virtual void GetWays(std::vector<Way>& out, const BBoxi& bbox) const = 0;

	/** Returns the center of available area */
	virtual Vector2i GetCenter() const {
		return Vector2i(0, 0);
	}

	/** Returns the bounding box of available area */
	virtual BBoxi GetBBox() const {
		return BBoxi::ForEarth();
	}
};

#endif
