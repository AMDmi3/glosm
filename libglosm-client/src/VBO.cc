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

#include <glosm/VBO.hh>

#include <glosm/util/gl.h>

IBO::IBO(const GLushort* data, int count) : size_(count) {
	glGenBuffers(1, &buffer_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*sizeof(GLushort), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IBO::~IBO() {
	glDeleteBuffers(1, &buffer_);
}

void IBO::Bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_);
}

void IBO::UnBind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

size_t IBO::GetSize() const {
	return size_;
}
