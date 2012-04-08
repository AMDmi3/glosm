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

#include "PixelBuffer.hh"

PixelBuffer::PixelBuffer(int width, int height, int bpp) : width_(width), height_(height), bpp_(bpp), pixels_(width * height * bpp) {
}

unsigned char* PixelBuffer::GetData() {
	return pixels_.data();
}

const unsigned char* PixelBuffer::GetRowPointer(int row, int offset) const {
	return pixels_.data() + row * width_ * bpp_ + offset * bpp_;
}

const unsigned char* PixelBuffer::GetReverseRowPointer(int row, int offset) const {
	return pixels_.data() + (height_ - 1 - row) * width_ * bpp_ + offset * bpp_;
}

int PixelBuffer::GetWidth() const {
	return width_;
}

int PixelBuffer::GetHeight() const {
	return height_;
}

int PixelBuffer::GetBPP() const {
	return bpp_;
}
