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

#include <glosm/MercatorProjection.hh>
#include <glosm/PreloadedXmlDatasource.hh>
#include <glosm/GeometryGenerator.hh>
#include <glosm/GeometryLayer.hh>
#include <glosm/OrthoViewer.hh>
#include <glosm/geomath.h>

#include "PBuffer.hh"
#include "PixelBuffer.hh"
#include "PngWriter.hh"

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <cstdio>

struct LevelInfo {
	int tiling;
	int flags;

	bool operator!= (const LevelInfo& other) const {
		return tiling != other.tiling || flags != other.flags;
	}
};

static LevelInfo LevelInfos[] = {
	{ 0, GeometryDatasource::GROUND }, /* 0 */
	{ 1, GeometryDatasource::GROUND }, /* 1 */
	{ 2, GeometryDatasource::GROUND }, /* 2 */
	{ 3, GeometryDatasource::GROUND }, /* 3 */
	{ 4, GeometryDatasource::GROUND }, /* 4 */
	{ 5, GeometryDatasource::GROUND }, /* 5 */
	{ 6, GeometryDatasource::GROUND }, /* 6 */
	{ 7, GeometryDatasource::GROUND }, /* 7 */
	{ 8, GeometryDatasource::GROUND }, /* 8 */
	{ 9, GeometryDatasource::GROUND }, /* 9 */
	{ 10, GeometryDatasource::GROUND }, /* 10 */
	{ 11, GeometryDatasource::EVERYTHING }, /* 11 */
	{ 12, GeometryDatasource::EVERYTHING }, /* 12 */
	{ 12, GeometryDatasource::EVERYTHING }, /* 13 */
	{ 12, GeometryDatasource::EVERYTHING }, /* 14 */
	{ 12, GeometryDatasource::EVERYTHING }, /* 15 */
	{ 13, GeometryDatasource::EVERYTHING }, /* 16 */
	{ 13, GeometryDatasource::EVERYTHING }, /* 17 */
	{ 14, GeometryDatasource::EVERYTHING }, /* 18 */
};

void usage(const char* progname) {
	fprintf(stderr, "Usage: %s [-0123456789] [-s skew] [-z minzoom] [-Z maxzoom] -x minlon -X maxlon -y minlat -Y maxlat infile.osm outdir\n", progname);
	exit(1);
}

int RenderTiles(PBuffer& pbuffer, OrthoViewer& viewer, GeometryLayer& layer, const char* target, float minlon, float minlat, float maxlon, float maxlat, int minzoom, int maxzoom, int pnglevel) {
	int x, y, zoom, ntiles = 0;
	PixelBuffer pixels(256, 256, 3);

	char path[FILENAME_MAX];
	snprintf(path, sizeof(path), "%s", target);
	mkdir(path, 0777);
	for (zoom = minzoom; zoom <= maxzoom; zoom++) {
		layer.SetLevel(LevelInfos[zoom].tiling);
		layer.SetFlags(LevelInfos[zoom].flags);
		if (zoom > 0 && LevelInfos[zoom-1] != LevelInfos[zoom])
			layer.Clear();

		int minxtile = (int)((minlon + 180.0)/360.0*powf(2.0, zoom));
		int maxxtile = (int)((maxlon + 180.0)/360.0*powf(2.0, zoom));
		int minytile = (int)((-mercator(maxlat/180.0*M_PI)/M_PI*180.0 + 180.0)/360.0*powf(2.0, zoom));
		int maxytile = (int)((-mercator(minlat/180.0*M_PI)/M_PI*180.0 + 180.0)/360.0*powf(2.0, zoom));

		snprintf(path, sizeof(path), "%s/%d", target, zoom);
		mkdir(path, 0777);
		for (x = minxtile; x <= maxxtile; ++x) {
			snprintf(path, sizeof(path), "%s/%d/%d", target, zoom, x);
			mkdir(path, 0777);
			for (y = minytile; y <= maxytile; ++y) {
				snprintf(path, sizeof(path), "%s/%d/%d/%d.png", target, zoom, x, y);

				BBoxi bbox = BBoxi::ForMercatorTile(zoom, x, y);
				viewer.SetBBox(bbox);

				BBoxi request_bbox = bbox;
				/* expand request 1km down for skewed buildings to show correctly
				 * that is, we assume maximum object height of 1km */
				request_bbox.bottom -= 1000.0 / WGS84_EARTH_EQ_LENGTH * 360.0 * GEOM_UNITSINDEGREE;

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				layer.GarbageCollect();
				layer.LoadArea(request_bbox, TileManager::SYNC);
				layer.Render(viewer);
				glFinish();

				pbuffer.GetPixels(pixels, 0, 0);

				PngWriter writer(path, 256, 256, pnglevel);
				writer.WriteImage(pixels, 0, 0);

				ntiles++;
			}
		}
	}

	return ntiles;
}

int real_main(int argc, char** argv) {
	const char* progname = argv[0];

	int pnglevel = 6;

	float minlat = 0.0f, maxlat = 0.0f, minlon = 0.0f, maxlon = 0.0f;
	int minzoom = 0, maxzoom = 18;

	float skew = 1.0f;

	int c;
	while ((c = getopt(argc, argv, "0123456789s:z:Z:x:X:y:Y:")) != -1) {
		switch (c) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
				  pnglevel = c - '0';
				  break;
		case 's': skew = strtof(optarg, NULL); break;
		case 'z': minzoom = strtol(optarg, NULL, 10); break;
		case 'Z': maxzoom = strtol(optarg, NULL, 10); break;
		case 'x': minlon = strtof(optarg, NULL); break;
		case 'X': maxlon = strtof(optarg, NULL); break;
		case 'y': minlat = strtof(optarg, NULL); break;
		case 'Y': maxlat = strtof(optarg, NULL); break;
		default:
			usage(progname);
		}
	}

	argc -= optind;
	argv += optind;

	if (minlon < -180.0f || maxlon > 180.0 || minlon > maxlon)
		usage(progname);
	if (minlat < -85.0f || maxlat > 85.0 || minlat > maxlat)
		usage(progname);
	if (minzoom < 0 || minzoom > maxzoom)
		usage(progname);
	if (skew <= 0.0)
		usage(progname);
	if (argc != 2)
		usage(progname);

	/* OpenGL init */
	PBuffer pbuffer(256, 256, 4);
	glClearColor(0.5, 0.5, 0.5, 0.0);

	/* glosm init */
	OrthoViewer viewer;
	viewer.SetSkew(skew);
	PreloadedXmlDatasource osm_datasource;

	fprintf(stderr, "Loading OSM data...\n");
	osm_datasource.Load(argv[0]);

	fprintf(stderr, "Creating geometry...\n");
	GeometryGenerator geometry_generator(osm_datasource);

	GeometryLayer layer(MercatorProjection(), geometry_generator);
	layer.SetSizeLimit(128*1024*1024);

	/* Rendering */
	fprintf(stderr, "Rendering...\n");

	struct timeval begin, end;

	gettimeofday(&begin, NULL);
	int ntiles = RenderTiles(pbuffer, viewer, layer, argv[1], minlon, minlat, maxlon, maxlat, minzoom, maxzoom, pnglevel);
	gettimeofday(&end, NULL);

	float dt = (float)(end.tv_sec - begin.tv_sec) + (float)(end.tv_usec - begin.tv_usec)/1000000.0f;

	fprintf(stderr, "%.2f seconds, %d tiles: %.2f tiles/sec\n", dt, ntiles, (float)ntiles/dt);

	return 0;
}

int main(int argc, char** argv) {
	try {
		return real_main(argc, argv);
	} catch (std::exception &e) {
		fprintf(stderr, "Exception: %s\n", e.what());
	} catch (...) {
		fprintf(stderr, "Unknown exception\n");
	}

	return 1;
}
