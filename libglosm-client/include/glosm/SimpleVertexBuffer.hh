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

#ifndef SIMPLEVERTEXBUFFER_HH
#define SIMPLEVERTEXBUFFER_HH

#if defined(__APPLE__)
#	include <OpenGL/gl.h>
#else
#	include <GL/gl.h>
#endif

#include <glosm/Math.hh>
#include <glosm/NonCopyable.hh>
#include <glosm/VBO.hh>
#include <glosm/Renderable.hh>

#include <memory>

/**
 * Static renderable geometry stored in VBOs
 */
class SimpleVertexBuffer : public Renderable, NonCopyable {
protected:
	std::auto_ptr<VBO> vertices_;
	std::auto_ptr<VBO> normals_;
	GLenum mode_;
	int size_;

public:
	enum Type {
		LINES,
		TRIANGLES,
		QUADS,
	};

public:
	SimpleVertexBuffer(Type type, Vector3f* vertices, int count);
	virtual ~SimpleVertexBuffer();

	int GetSize() const;

	virtual void Render();
};

#endif
