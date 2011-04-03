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

#ifndef SRTMDATASOURCE_HH
#define SRTMDATASOURCE_HH

#include <glosm/HeightmapDatasource.hh>

#include <stdint.h>
#include <pthread.h>

#include <vector>
#include <map>

class SRTMDatasource : public HeightmapDatasource {
protected:
	struct ChunkId {
		short lon;
		short lat;

		ChunkId(short lo, short la): lon(lo), lat(la) {
		}

		bool operator< (const ChunkId& other) const {
			return lon < other.lon || (lon == other.lon && lat < other.lat);
		}
	};

	struct Chunk {
		int generation;
		std::vector<int16_t> data;
	};

protected:
	typedef std::map<ChunkId, Chunk> ChunksMap;

protected:
	const char* storage_path_;
	int generation_;

	pthread_mutex_t mutex_;

	mutable ChunksMap chunks_;

protected:
	Chunk& RequireChunk(int lon, int lat);
	osmint_t GetPointHeight(int x, int y);

public:
	SRTMDatasource(const char* storage_path);
	virtual ~SRTMDatasource();

	virtual void GetHeightmap(const BBoxi& bbox, int extramargin, Heightmap& out);
	virtual osmint_t GetHeight(const Vector2i& where);
};

#endif
