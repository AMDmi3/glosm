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

#ifndef PIXELBUFFER_HH
#define PIXELBUFFER_HH

#include <vector>

class PixelBuffer {
protected:
	int width_;
	int height_;
	int bpp_;
	std::vector<unsigned char> pixels_;

public:
	PixelBuffer(int width, int height, int bpp);

	unsigned char* GetData();

	const unsigned char* GetRowPointer(int row, int offset) const;
	const unsigned char* GetReverseRowPointer(int row, int offset) const;

	int GetWidth() const;
	int GetHeight() const;
	int GetBPP() const;
};

#endif
