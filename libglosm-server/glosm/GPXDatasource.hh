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

#ifndef GPXDATASOURCE_HH
#define GPXDATASOURCE_HH

#include <glosm/Math.hh>
#include <glosm/BBox.hh>

#include <vector>

/**
 * Abstract base class for sources of GPX data.
 */
class GPXDatasource {
public:
	virtual void GetPoints(std::vector<Vector3i>& out, const BBoxi& bbox) const = 0;
};

#endif
