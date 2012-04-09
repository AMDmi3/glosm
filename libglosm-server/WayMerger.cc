/*
 * Copyright (C) 2010-2012 Dmitry Marakasov
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

#include <cassert>
#include <algorithm>

#include <glosm/OsmDatasource.hh>
#include <glosm/WayMerger.hh>

WayMerger::WayMerger() {
}

void WayMerger::AddWay(const OsmDatasource::Way::NodesList& nodes) {
	way_by_node.insert(std::make_pair(nodes.front(), &nodes));
	way_by_node_rev.insert(std::make_pair(nodes.back(), &nodes));
}

bool WayMerger::GetNextWay(OsmDatasource::Way::NodesList& outnodes) {
	OsmDatasource::Way::NodesList tempnodes;

	std::back_insert_iterator<OsmDatasource::Way::NodesList> inserter(tempnodes);

	const OsmDatasource::Way::NodesList* next_chain;
	while (!way_by_node.empty()) {
		if (tempnodes.empty()) {
			/* no current nodes, add just any chain */
			next_chain = PickChain();
			assert(next_chain);
			std::copy(next_chain->begin(), next_chain->end(), inserter);
		} else {
			/* find next chain, first try front map, then reverse */
			if ((next_chain = PickChain(tempnodes.back())) != NULL) {
				std::copy(++next_chain->begin(), next_chain->end(), inserter);
			} else if ((next_chain = PickChain(tempnodes.back(), true)) != NULL) {
				std::copy(++next_chain->rbegin(), next_chain->rend(), inserter);
			} else {
				/* no suitable next chain, drop partial way */
				/* XXX: (or, we can return it if we need unclosed ways too */
				tempnodes.clear();
			}
		}

		/* check if we have complete cycle */
		if (!tempnodes.empty() && tempnodes.front() == tempnodes.back()) {
			outnodes.swap(tempnodes);
			return true;
		}
	}

	return false;
}

const OsmDatasource::Way::NodesList* WayMerger::PickChain(osmid_t id, bool reverse) {
	WayByTipMap& primary = reverse ? way_by_node_rev : way_by_node;
	WayByTipMap& secondary = reverse ? way_by_node : way_by_node_rev;

	/* find chain starting with given id in primary map */
	WayByTipMap::iterator primary_iterator = ((id == 0) ? primary.begin() : primary.find(id));
	if (primary_iterator == primary.end())
		return NULL;

	/* save chain poiter which we will return */
	const OsmDatasource::Way::NodesList* ret = primary_iterator->second;

	/* find the same chain in the other map */
	std::pair<WayByTipMap::iterator, WayByTipMap::iterator> range =
		secondary.equal_range(reverse ? ret->front() : ret->back());

	WayByTipMap::iterator secondary_iterator;
	for (secondary_iterator = range.first; secondary_iterator != range.second; ++secondary_iterator) {
		if (secondary_iterator->second == ret) {
			break;
		}
	}

	assert(secondary_iterator != range.second);

	/* delete chain references from both maps */
	secondary.erase(secondary_iterator);
	primary.erase(primary_iterator);

	return ret;
}
