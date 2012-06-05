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

#include <glosm/SRTMDatasource.hh>
#include <glosm/Exception.hh>
#include <glosm/Guard.hh>
#include <glosm/Misc.hh>

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
#include <cerrno>

enum {
	FILE_HEIGHT = 1201,
	FILE_WIDTH = 1201,

	DATA_HEIGHT = 1200,
	DATA_WIDTH = 1200,
};

SRTMDatasource::SRTMDatasource(const char* storage_path) : storage_path_(storage_path) {
	generation_ = 0;
	int errn;
	if ((errn = pthread_mutex_init(&mutex_, 0)) != 0)
		throw SystemError(errn) << "pthread_mutex_init failed";
}

SRTMDatasource::~SRTMDatasource() {
	pthread_mutex_destroy(&mutex_);
}

SRTMDatasource::Chunk& SRTMDatasource::RequireChunk(int lon, int lat) const {
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
					size_t toread = 2 * DATA_WIDTH;
					char* readptr = reinterpret_cast<char*>(current);

					while (toread > 0) {
						ssize_t nread = read(f, readptr, toread);

						if (nread == -1 && errno == EINTR)
							continue;
						else if (nread == -1)
							throw SystemError() << "read error on SRTM file " << filename.str();
						else if (nread == 0)
							throw Exception() << "unexpected EOF in SRTM file " << filename.str();

						toread -= nread;
						readptr += nread;
					}

					/* SRTM data is in big-endian format, convert it if needed */
					if (!IsBigEndian()) {
						for (uint16_t* val = (uint16_t*)current; val < (uint16_t*)current + DATA_WIDTH; ++val)
							*val = (*val >> 8) | (*val << 8);
					}

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

osmint_t SRTMDatasource::GetPointHeight(int x, int y) const {
	int xchunk = x / DATA_WIDTH;
	int ychunk = y / DATA_HEIGHT;

	int pos = x - xchunk * DATA_WIDTH;
	int line = y - ychunk * DATA_HEIGHT;

	Chunk& chunk = RequireChunk(xchunk, ychunk);

	return chunk.data[(DATA_HEIGHT - 1 - line) * DATA_WIDTH + pos] * GEOM_UNITSINMETER;
}

void SRTMDatasource::GetHeightmap(const BBoxi& bbox, int extramargin, Heightmap& out) const {
	Guard guard(mutex_);

	BBox<int> srtm_bbox; /* bbox in srtm point numbers, zero-based at bottom left corner */
	BBox<int> srtm_chunks; /* bbox in srtm chunk numbers, zero-based at bottom left corner */

	srtm_bbox.left = (int)floor(((double)bbox.left / (double)GEOM_UNITSINDEGREE + 180.0) * (double)DATA_WIDTH) - extramargin;
	srtm_bbox.bottom = (int)floor(((double)bbox.bottom / (double)GEOM_UNITSINDEGREE + 90.0) * (double)DATA_HEIGHT) - extramargin;
	srtm_bbox.right = (int)ceil(((double)bbox.right / (double)GEOM_UNITSINDEGREE + 180.0) * (double)DATA_WIDTH) + extramargin;
	srtm_bbox.top = (int)ceil(((double)bbox.top / (double)GEOM_UNITSINDEGREE + 90.0) * (double)DATA_HEIGHT) + extramargin;

	srtm_chunks.left = srtm_bbox.left / DATA_WIDTH;
	srtm_chunks.bottom = srtm_bbox.bottom / DATA_HEIGHT;
	srtm_chunks.right = srtm_bbox.right / DATA_WIDTH;
	srtm_chunks.top = srtm_bbox.top / DATA_HEIGHT;

	out.bbox.left = (osmint_t)round(((double)srtm_bbox.left / (double)DATA_WIDTH - 180.0) * (double)GEOM_UNITSINDEGREE);
	out.bbox.bottom = (osmint_t)round(((double)srtm_bbox.bottom / (double)DATA_HEIGHT - 90.0) * (double)GEOM_UNITSINDEGREE);
	out.bbox.right = (osmint_t)round(((double)srtm_bbox.right / (double)DATA_WIDTH - 180.0) * (double)GEOM_UNITSINDEGREE);
	out.bbox.top = (osmint_t)round(((double)srtm_bbox.top / (double)DATA_HEIGHT - 90.0) * (double)GEOM_UNITSINDEGREE);

	int& width = out.width = srtm_bbox.right - srtm_bbox.left + 1;
	int& height = out.height = srtm_bbox.top - srtm_bbox.bottom + 1;

	out.points.resize(width * height);

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
					out.points[(ychunk * DATA_HEIGHT - srtm_bbox.bottom + line) * width + (xchunk * DATA_WIDTH - srtm_bbox.left + pos)] = current;
				}
			}
		}
	}
}

osmint_t SRTMDatasource::GetHeight(const Vector2i& where) const {
	Guard guard(mutex_);

	BBox<int> srtm_bbox; /* bbox in srtm point numbers, zero-based at bottom left corner */
	BBox<int> srtm_chunks; /* bbox in srtm chunk numbers, zero-based at bottom left corner */
	BBoxi real_bbox;

	srtm_bbox.left = (int)floor(((double)where.x / (double)GEOM_UNITSINDEGREE + 180.0) * (double)DATA_WIDTH);
	srtm_bbox.bottom = (int)floor(((double)where.y / (double)GEOM_UNITSINDEGREE + 90.0) * (double)DATA_HEIGHT);
	srtm_bbox.right = srtm_bbox.left + 1;
	srtm_bbox.top = srtm_bbox.bottom + 1;

	real_bbox.left = (osmint_t)round(((double)srtm_bbox.left / (double)DATA_WIDTH - 180.0) * (double)GEOM_UNITSINDEGREE);
	real_bbox.bottom = (osmint_t)round(((double)srtm_bbox.bottom / (double)DATA_HEIGHT - 90.0) * (double)GEOM_UNITSINDEGREE);
	real_bbox.right = (osmint_t)round(((double)srtm_bbox.right / (double)DATA_WIDTH - 180.0) * (double)GEOM_UNITSINDEGREE);
	real_bbox.top = (osmint_t)round(((double)srtm_bbox.top / (double)DATA_HEIGHT - 90.0) * (double)GEOM_UNITSINDEGREE);

	double kx = (double)(where.x - real_bbox.left)/(double)(real_bbox.right - real_bbox.left);
	double ky = (double)(where.y - real_bbox.bottom)/(double)(real_bbox.top - real_bbox.bottom);

	/*
	 * here we take into account that our heightmap is split
	 * into triangles like this:
	 * +--+
	 * | /|
	 * |/ |
	 * +--+
	 * but "true" height would be 4-point interpolation
	 */

	double height;
	if (kx < ky) {
		height = (double)GetPointHeight(srtm_bbox.left, srtm_bbox.bottom) * (1 - ky) +
		         (double)GetPointHeight(srtm_bbox.right, srtm_bbox.top) * (kx) +
		         (double)GetPointHeight(srtm_bbox.left, srtm_bbox.top) * (ky - kx);
	} else {
		height = (double)GetPointHeight(srtm_bbox.left, srtm_bbox.bottom) * (1 - kx) +
		         (double)GetPointHeight(srtm_bbox.right, srtm_bbox.top) * (ky) +
		         (double)GetPointHeight(srtm_bbox.right, srtm_bbox.bottom) * (kx - ky);
	}

	return (osmint_t)round(height);
}
