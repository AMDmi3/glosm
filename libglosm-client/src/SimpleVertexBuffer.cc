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

#include <glosm/SimpleVertexBuffer.hh>

#include <glosm/VBO.hh>

#include <glosm/util/gl.h>

#include <stdexcept>
#include <cassert>
#include <vector>

SimpleVertexBuffer::SimpleVertexBuffer(Type type, Vector3f* vertices, int count): vertices_(new VBO<Vector3f>(GL_ARRAY_BUFFER, vertices, count)), size_(count) {
	/* calculate normals; not sure if that logically belongs
	 * here, but for now it's useful */
	/* @todo use single VBO for both positions and normals */
	if (type == TRIANGLES || type == QUADS) {
		int nvert = type == TRIANGLES ? 3 : 4;

		std::vector<Vector3f> normals;
		normals.resize(count);
		for (int i = 0; i+nvert-1 < count; i += nvert) {
			Vector3f normal = -(vertices[i+1] - vertices[i]).CrossProduct(vertices[i+1] - vertices[i+2]).Normalized();
			normals[i] = normals[i+1] = normals[i+2] = normal;
			if (nvert == 4)
				normals[i+3] = normal;

		}

		normals_.reset(new VBO<Vector3f>(GL_ARRAY_BUFFER, normals.data(), normals.size()));

		assert(vertices_->GetSize() == normals_->GetSize());
	}

	switch (type) {
	case POINTS: mode_ = GL_POINTS; break;
	case LINES: mode_ = GL_LINES; break;
	case TRIANGLES: mode_ = GL_TRIANGLES; break;
#if !defined(WITH_GLES) && !defined(WITH_GLES2)
	case QUADS: mode_ = GL_QUADS; break;
#endif
	default:
		throw std::logic_error("unhandled geometry type");
	}
}

SimpleVertexBuffer::~SimpleVertexBuffer() {
}

int SimpleVertexBuffer::GetSize() const {
	return size_;
}

void SimpleVertexBuffer::Render() {
	glEnableClientState(GL_VERTEX_ARRAY);
	vertices_->Bind();
	glVertexPointer(3, GL_FLOAT, 0, 0);

	if (normals_.get()) {
		glEnableClientState(GL_NORMAL_ARRAY);
		normals_->Bind();
		glNormalPointer(GL_FLOAT, 0, 0);
	}

	glDrawArrays(mode_, 0, vertices_->GetSize());

	if (normals_.get())
		glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
