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

#ifndef SRTMDATASOURCE_HH
#define SRTMDATASOURCE_HH

#include <glosm/HeightmapDatasource.hh>

class SRTMDatasource : public HeightmapDatasource {
public:
	SRTMDatasource(const char* storage_path);
	virtual ~SRTMDatasource();

	virtual void GetHeights(std::vector<osmint_t>& out, BBoxi& outbbox, Vector2i& res, const BBoxi& bbox);
};

#endif
