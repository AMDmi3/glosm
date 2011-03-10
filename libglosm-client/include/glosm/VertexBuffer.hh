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

#ifndef VERTEXBUFFER_HH
#define VERTEXBUFFER_HH

#include <glosm/NonCopyable.hh>
#include <glosm/Math.hh>

#include <glosm/util/gl.h>

#include <sys/types.h>

#include <vector>
#include <memory>
#include <stdexcept>
#include <cassert>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/**
 * Vertex Buffer Object
 *
 * This class is mainly a wrapper around OpenGL VBO's. However,
 * since delayed VBO creation is common in glosm, this may also
 * be used as a vector of vertex data, which turns into VBO on
 * first Bind() or Freeze() call.
 */
template<class T>
class VertexBuffer : private NonCopyable {
public:
	typedef std::vector<T> DataVector;

protected:
	GLuint buffer_id_;
	size_t size_;
	GLenum type_;
	std::auto_ptr<DataVector> ram_data_;

protected:
	void CreateBufferObject(const T* data, int count) {
		glGenBuffers(1, &buffer_id_);
		glBindBuffer(type_, buffer_id_);
		glBufferData(type_, count * sizeof(T), data, GL_STATIC_DRAW);
		glBindBuffer(type_, 0);
		size_ = count;
	}

public:
	/**
	 * Constructs dynamic vertex buffer
	 *
	 * Vertex buffer constructed this way is ready to be filled
	 * with data and be generally used as vector of T's via
	 * Data() call. OpenGL buffers are not created until first
	 * Bind() or Freeze() calls
	 *
	 * @param type buffer type (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER)
	 */
	VertexBuffer(GLenum type): buffer_id_(0), size_(0), type_(type), ram_data_(new DataVector) {
	}

	/**
	 * Constructs static vertex buffer
	 *
	 * Constructs OpenGL VBO directly from preexisting data.
	 *
	 * @param type buffer type (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER)
	 * @param data pointer to array of data items
	 * @param count number of items
	 */
	VertexBuffer(GLenum type, const T* data, int count): type_(type) {
		CreateBufferObject(data, count);
	}

	/**
	 * Destructor
	 */
	~VertexBuffer() {
		if (buffer_id_)
			glDeleteBuffers(1, &buffer_id_);
	}

	/**
	 * Gives access to vector of vertex data
	 *
	 * @return reference to data vector
	 */
	DataVector& Data() {
		assert(ram_data_.get());
		return *ram_data_;
	}

	/**
	 * Forcibly converts data into OpenGL VBO
	 *
	 * @return true if VBO was created
	 */
	bool Freeze() {
		if (ram_data_.get()) {
			CreateBufferObject(ram_data_->data(), ram_data_->size());
			ram_data_.reset(NULL);
			return true;
		}
		return false;
	}

	/**
	 * Binds OpenGL buffer
	 *
	 * Creates it from vertex data if necessary
	 */
	void Bind() {
		Freeze();

		assert(buffer_id_);
		glBindBuffer(type_, buffer_id_);
	}

	/**
	 * Unbinds OpenGL buffer
	 */
	void UnBind() {
		glBindBuffer(type_, 0);
	}

	/**
	 * Returns number of items in VBO
	 */
	size_t GetSize() const {
		if (ram_data_.get())
			return ram_data_->size();
		return size_;
	}

	/**
	 * Returns size of VBO in bytes
	 */
	size_t GetFootprint() const {
		if (ram_data_.get())
			return sizeof(T) * ram_data_->size();
		return sizeof(T) * size_;
	}
};

#endif
