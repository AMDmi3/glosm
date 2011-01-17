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

#include <GL/gl.h>

#include "NonCopyable.hh"
#include "Math.hh"

/**
 * Wrapper around OpenGL vertex buffer object
 */
class VBO : public NonCopyable {
protected:
	GLuint buffer_;
	size_t size_;

public:
	VBO(const Vector3f* data, int count);
	~VBO();

	void Bind() const;
	size_t GetSize() const;
};

#endif
