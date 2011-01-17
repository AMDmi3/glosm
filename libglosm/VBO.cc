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
#	include <OpenGL/glext.h>
#else
#	include <GL/gl.h>
#	include <GL/glext.h>
#endif

#include "VBO.hh"

VBO::VBO(const Vector3f* data, int count) : size_(count) {
	glGenBuffers(1, &buffer_);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);
	glBufferData(GL_ARRAY_BUFFER, count*sizeof(Vector3f), data, GL_STATIC_DRAW);
}

VBO::~VBO() {
	glDeleteBuffers(1, &buffer_);
}

void VBO::Bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);
}

size_t VBO::GetSize() const {
	return size_;
}
