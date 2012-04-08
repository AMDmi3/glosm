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

#ifndef PBUFFER_HH
#define PBUFFER_HH

#include <glosm/Exception.hh>

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <vector>

class PBufferException: public Exception {
};

class PixelBuffer;

class PBuffer {
protected:
	int width_;
	int height_;

	Display* display_;
	GLXContext context_;
	GLXPbuffer pbuffer_;

public:
	PBuffer(int width, int height, int samples);
	~PBuffer();

	void GetPixels(PixelBuffer& buffer, int x, int y);

	bool IsDirect() const;
};

#endif
