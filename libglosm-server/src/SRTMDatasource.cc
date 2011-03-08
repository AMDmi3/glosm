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

#include <glosm/SRTMDatasource.hh>
#include <glosm/Exception.hh>

#include <glosm/geomath.h>

#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <cstdio>

enum {
	FILE_HEIGHT = 1201,
	FILE_WIDTH = 1201,

	DATA_HEIGHT = 1200,
	DATA_WIDTH = 1200,
};

SRTMDatasource::SRTMDatasource(const char* storage_path) : storage_path_(storage_path) {
	generation_ = 0;
}

SRTMDatasource::~SRTMDatasource() {
}

SRTMDatasource::Chunk& SRTMDatasource::RequireChunk(int lon, int lat) {
	lon -= 180;
	lat -= 90;

	ChunksMap::iterator chunk = chunks_.find(ChunkId(lon, lat));
	if (chunk == chunks_.end()) {
		std::pair<ChunksMap::iterator, bool> p = chunks_.insert(std::make_pair(ChunkId(lon, lat), Chunk()));
		assert(p.second);
		chunk = p.first;
	}

	chunk->second.generation = ++generation_;

	/* hardcoded cleanup routine; make this customizable like in TileManager */
	if (chunks_.size() > 32) {
		std::multimap<int, ChunksMap::iterator> gensorted;
		for (ChunksMap::iterator i = chunks_.begin(); i != chunks_.end(); ++i)
			gensorted.insert(std::make_pair(i->second.generation, i));

		for (unsigned int i = 24; i < chunks_.size(); ++i)
			gensorted.erase(gensorted.begin());
	}

	try {
		if (chunk->second.data.empty()) {
			chunk->second.data.resize(DATA_HEIGHT * DATA_WIDTH);

			std::stringstream filename;
			filename << storage_path_ << "/" << std::setfill('0')
				<< (lat < 0 ? 'E' : 'N') << std::setw(2) << abs(lat)
				<< (lon < 0 ? 'W' : 'E') << std::setw(3) << abs(lon) << ".hgt";

			int f;
			if ((f = open(filename.str().c_str(), O_RDONLY)) == -1)
				throw SystemError() << "cannot open SRTM file " << filename.str();

			try {
				int16_t* current = chunk->second.data.data();

				for (int line = 0; line < DATA_HEIGHT; line++) {
					int nread;
					if ((nread = read(f, current, 2 * DATA_WIDTH)) == -1)
						throw SystemError() << "read error on SRTM file " << filename.str();

					if (nread != 2 * DATA_WIDTH)
						throw Exception() << "short read on SRTM file " << filename.str();

#if BYTE_ORDER != BIG_ENDIAN
					/* SRTM data is in big-endian format */
					for (uint16_t* val = (uint16_t*)current; val < (uint16_t*)current + DATA_WIDTH; ++val)
						*val = (*val >> 8) | (*val << 8);
#endif

					if (lseek(f, 2 * (FILE_WIDTH - DATA_WIDTH), SEEK_CUR) == -1)
						throw SystemError() << "cannot seek SRTM file " << filename.str();

					current += DATA_WIDTH;
				}
			} catch (...) {
				close(f);
				throw;
			}

			close(f);
		}
	} catch (Exception& e) {
		/* if the problem is in our code, just display warning.
		 * returned chunk will be legal, but it will have zeroed
		 * or partial data, which is better than just dying */
		fprintf(stderr, "warning: %s\n", e.what());
	} catch (...) {
		chunks_.erase(chunk);
		throw;
	}

	return chunk->second;
}

void SRTMDatasource::GetHeights(std::vector<osmint_t>& out, BBoxi& outbbox, Vector2<int>& res, const BBoxi& bbox) {
	BBox<int> srtm_bbox; /* bbox in srtm point numbers, zero-based at bottom left corner */
	BBox<int> srtm_chunks; /* bbox in srtm chunk numbers, zero-based at bottom left corner */

	srtm_bbox.left = (int)floor(((double)bbox.left / (double)GEOM_UNITSINDEGREE + 180.0) * (double)DATA_WIDTH);
	srtm_bbox.bottom = (int)floor(((double)bbox.bottom / (double)GEOM_UNITSINDEGREE + 90.0) * (double)DATA_HEIGHT);
	srtm_bbox.right = (int)ceil(((double)bbox.right / (double)GEOM_UNITSINDEGREE + 180.0) * (double)DATA_WIDTH);
	srtm_bbox.top = (int)ceil(((double)bbox.top / (double)GEOM_UNITSINDEGREE + 90.0) * (double)DATA_HEIGHT);

	srtm_chunks.left = srtm_bbox.left / DATA_WIDTH;
	srtm_chunks.bottom = srtm_bbox.bottom / DATA_HEIGHT;
	srtm_chunks.right = srtm_bbox.right / DATA_WIDTH;
	srtm_chunks.top = srtm_bbox.top / DATA_HEIGHT;

	outbbox.left = (osmint_t)round(((double)srtm_bbox.left / (double)DATA_WIDTH - 180.0) * (double)GEOM_UNITSINDEGREE);
	outbbox.bottom = (osmint_t)round(((double)srtm_bbox.bottom / (double)DATA_HEIGHT - 90.0) * (double)GEOM_UNITSINDEGREE);
	outbbox.right = (osmint_t)round(((double)srtm_bbox.right / (double)DATA_WIDTH - 180.0) * (double)GEOM_UNITSINDEGREE);
	outbbox.top = (osmint_t)round(((double)srtm_bbox.top / (double)DATA_HEIGHT - 90.0) * (double)GEOM_UNITSINDEGREE);

	res.x = srtm_bbox.right - srtm_bbox.left + 1;
	res.y = srtm_bbox.top - srtm_bbox.bottom + 1;

	out.resize(res.x * res.y);

	BBox<int> chunk_bbox;
	for (int ychunk = srtm_chunks.bottom; ychunk <= srtm_chunks.top; ++ychunk) {
		for (int xchunk = srtm_chunks.left; xchunk <= srtm_chunks.right; ++xchunk) {
			chunk_bbox.left = std::max(xchunk * DATA_WIDTH, srtm_bbox.left);
			chunk_bbox.bottom = std::max(ychunk * DATA_HEIGHT, srtm_bbox.bottom);
			chunk_bbox.right = std::min((xchunk + 1) * DATA_WIDTH - 1, srtm_bbox.right);
			chunk_bbox.top = std::min((ychunk + 1) * DATA_HEIGHT - 1, srtm_bbox.top);

			chunk_bbox -= Vector2<int>(xchunk * DATA_WIDTH, ychunk * DATA_HEIGHT);

			Chunk& chunk = RequireChunk(xchunk, ychunk);
			for (int line = chunk_bbox.bottom; line <= chunk_bbox.top; ++line) {
				for (int pos = chunk_bbox.left; pos <= chunk_bbox.right; ++pos) {
					osmint_t current = chunk.data[(DATA_HEIGHT - 1 - line) * DATA_WIDTH + pos] * GEOM_UNITSINMETER;
					out[(ychunk * DATA_HEIGHT - srtm_bbox.bottom + line) * res.x + (xchunk * DATA_WIDTH - srtm_bbox.left + pos)] = current;
				}
			}
		}
	}
}

void SRTMDatasource::GetHeightBounds(const BBoxi& bbox, osmint_t& low, osmint_t& high) {
	BBox<int> srtm_bbox; /* bbox in srtm point numbers, zero-based at bottom left corner */
	BBox<int> srtm_chunks; /* bbox in srtm chunk numbers, zero-based at bottom left corner */

	srtm_bbox.left = (int)floor(((double)bbox.left / (double)GEOM_UNITSINDEGREE + 180.0) * (double)DATA_WIDTH);
	srtm_bbox.bottom = (int)floor(((double)bbox.bottom / (double)GEOM_UNITSINDEGREE + 90.0) * (double)DATA_HEIGHT);
	srtm_bbox.right = (int)ceil(((double)bbox.right / (double)GEOM_UNITSINDEGREE + 180.0) * (double)DATA_WIDTH);
	srtm_bbox.top = (int)ceil(((double)bbox.top / (double)GEOM_UNITSINDEGREE + 90.0) * (double)DATA_HEIGHT);

	srtm_chunks.left = srtm_bbox.left / DATA_WIDTH;
	srtm_chunks.bottom = srtm_bbox.bottom / DATA_HEIGHT;
	srtm_chunks.right = srtm_bbox.right / DATA_WIDTH;
	srtm_chunks.top = srtm_bbox.top / DATA_HEIGHT;

	BBox<int> chunk_bbox;
	bool first = true;
	for (int ychunk = srtm_chunks.bottom; ychunk <= srtm_chunks.top; ++ychunk) {
		for (int xchunk = srtm_chunks.left; xchunk <= srtm_chunks.right; ++xchunk) {
			chunk_bbox.left = std::max(xchunk * DATA_WIDTH, srtm_bbox.left);
			chunk_bbox.bottom = std::max(ychunk * DATA_HEIGHT, srtm_bbox.bottom);
			chunk_bbox.right = std::min((xchunk + 1) * DATA_WIDTH - 1, srtm_bbox.right);
			chunk_bbox.top = std::min((ychunk + 1) * DATA_HEIGHT - 1, srtm_bbox.top);

			chunk_bbox -= Vector2<int>(xchunk * DATA_WIDTH, ychunk * DATA_HEIGHT);

			Chunk& chunk = RequireChunk(xchunk, ychunk);
			for (int line = chunk_bbox.bottom; line <= chunk_bbox.top; ++line) {
				for (int pos = chunk_bbox.left; pos <= chunk_bbox.right; ++pos) {
					osmint_t current = chunk.data[(DATA_HEIGHT - 1 - line) * DATA_WIDTH + pos] * GEOM_UNITSINMETER;

					if (first) {
						low = high = current;
						first = false;
					} else {
						if (current < low)
							low = current;
						if (current > high)
							high = current;
					}
				}
			}
		}
	}
}
