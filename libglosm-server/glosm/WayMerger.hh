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

#ifndef WAYMERGER_HH
#define WAYMERGER_HH

#include <map>
#include <vector>

#include <glosm/osmtypes.h>

/**
 * Class that merges complete ways from parts
 *
 * @note this currently only works with closed ways, but a
 *       flag(s) may be added to support e.g routes
 */
class WayMerger {
protected:
	typedef std::multimap<osmid_t, const OsmDatasource::Way::NodesList*> WayByTipMap;

	typedef std::vector<osmid_t> NodeChain;
	typedef std::vector<NodeChain> NodeChainVector;

protected:
	WayByTipMap way_by_node;
	WayByTipMap way_by_node_rev;

public:
	WayMerger();

	void AddWay(const OsmDatasource::Way::NodesList& nodes);

	bool GetNextWay(OsmDatasource::Way::NodesList& outnodes);

protected:
	const OsmDatasource::Way::NodesList* PickChain(osmid_t id = 0, bool reverse = false);
};

#endif
