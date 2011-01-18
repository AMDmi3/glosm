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

#define GL_GLEXT_PROTOTYPES

#if defined(__APPLE__)
#	include <OpenGL/gl.h>
#else
#	include <GL/gl.h>
#endif

#include <glosm/GeometryTile.hh>

#include <glosm/Projection.hh>
#include <glosm/Geometry.hh>
#include <glosm/GeometryDatasource.hh>
#include <glosm/SimpleVertexBuffer.hh>

GeometryTile::GeometryTile(const Projection& projection, const GeometryDatasource& ds, const Vector2i& ref, const BBoxi& bbox = BBoxi::Full()) : Tile(ref) {
	Geometry g;
	ds.GetGeometry(g, bbox);

	std::vector<Vector3f> translated;
	projection.ProjectPoints(g.GetLines(), ref, translated);
	lines_.reset(new SimpleVertexBuffer(SimpleVertexBuffer::LINES, translated.data(), translated.size()));

	translated.clear();
	projection.ProjectPoints(g.GetTriangles(), ref, translated);
	triangles_.reset(new SimpleVertexBuffer(SimpleVertexBuffer::TRIANGLES, translated.data(), translated.size()));

	translated.clear();
	projection.ProjectPoints(g.GetQuads(), ref, translated);
	quads_.reset(new SimpleVertexBuffer(SimpleVertexBuffer::QUADS, translated.data(), translated.size()));
}

GeometryTile::~GeometryTile() {
}

void GeometryTile::Render() const {
	glDepthFunc(GL_LESS);

	/* lines */
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

	/*glEnable(GL_LINE_STIPPLE);
	glLineStipple(1.0, 0xCCCC);
	glDisable(GL_LINE_STIPPLE);
	*/

	lines_->Render();

	/* polygons */
	glPolygonOffset(1.0, 1.0);
	glEnable(GL_POLYGON_OFFSET_FILL);

	/* zpass */
	/*glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	triangles_->Render();
	quads_->Render();

	glDepthFunc(GL_EQUAL);*/

	/* objects */
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	triangles_->Render();
	quads_->Render();

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

	glDisable(GL_POLYGON_OFFSET_FILL);
}
