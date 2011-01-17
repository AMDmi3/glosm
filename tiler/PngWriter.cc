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

#include "PngWriter.hh"

#include "PixelBuffer.hh"

#include <GL/gl.h>
#include <png.h>

static void png_error_fn(png_struct*, const char* e) {
	throw PngWriterException(std::string("png write error: ") + std::string(e));
}

PngWriter::PngWriter(const char* filename, int width, int height, int compression): width_(width), height_(height) {
	if ((png_ptr_ = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_error_fn, NULL)) == NULL)
		throw PngWriterException("png_create_write_struct failed");

	png_set_compression_level(png_ptr_, compression);

	if ((info_ptr_ = png_create_info_struct(png_ptr_)) == NULL) {
		png_destroy_write_struct(&png_ptr_, NULL);
		throw PngWriterException("png_create_info_struct failed");
	}

	if ((file_ = fopen(filename, "wb")) == NULL) {
		png_destroy_write_struct(&png_ptr_, &info_ptr_);
		throw PngWriterException(std::string("cannot open output png file: ") + filename);
	}

	png_init_io(png_ptr_, file_);

	png_set_IHDR(png_ptr_, info_ptr_, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr_, info_ptr_);
}

PngWriter::~PngWriter() {
	png_write_end(png_ptr_, NULL);
	png_destroy_write_struct(&png_ptr_, &info_ptr_);
	fclose(file_);
}

void PngWriter::WriteImage(const PixelBuffer& buffer, int x, int y) {
	if (x + width_ > buffer.GetWidth())
		throw std::logic_error("output image is wider than input buffer");

	if (y + height_ > buffer.GetHeight())
		throw std::logic_error("output image is higher than input buffer");

	png_bytep* row_pointers = new png_bytep[height_];

	/* ugly cast is hack for png interface which takes non-const png_pytepp */
	for (int i = 0; i < height_; ++i)
		row_pointers[i] = const_cast<png_byte*>(buffer.GetReverseRowPointer(y + i, x));

	png_write_image(png_ptr_, row_pointers);

	delete[] row_pointers;
}
