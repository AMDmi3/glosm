/*
 * Copyright (C) 2010-2012 Dmitry Marakasov
 *
 * This file is part of glosm.
 *
 * glosm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * glosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with glosm.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PNGWRITER_HH
#define PNGWRITER_HH

#include <glosm/Exception.hh>

#include <png.h>

class PixelBuffer;

class PngWriterException : public Exception {
};

class PngWriter {
protected:
	FILE* file_;
	png_structp png_ptr_;
	png_infop info_ptr_;
	int width_;
	int height_;

public:
	PngWriter(const char* filename, int width, int height, int compression);
	virtual ~PngWriter();

	void WriteImage(const PixelBuffer& buffer, int x, int y);
};

#endif
