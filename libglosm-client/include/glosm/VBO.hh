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

#ifndef VBO_HH
#define VBO_HH

#include <glosm/NonCopyable.hh>
#include <glosm/Math.hh>

#include <glosm/util/gl.h>

#include <sys/types.h>

/**
 * Wrapper around OpenGL vertex buffer object
 */
template<class T>
class VBO : private NonCopyable {
protected:
	GLuint buffer_;
	size_t size_;
	GLenum type_;

public:
	VBO(GLenum type, const T* data, int count): type_(type), size_(count) {
		glGenBuffers(1, &buffer_);
		glBindBuffer(type, buffer_);
		glBufferData(type, count*sizeof(T), data, GL_STATIC_DRAW);
		glBindBuffer(type, 0);
	}

	~VBO() {
		glDeleteBuffers(1, &buffer_);
	}

	void Bind() const {
		glBindBuffer(type_, buffer_);
	}

	void UnBind() const {
		glBindBuffer(type_, 0);
	}

	size_t GetSize() const {
		return size_;
	}
};

#endif
