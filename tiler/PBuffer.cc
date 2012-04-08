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

#include <err.h>
#include <string.h>

#include "PBuffer.hh"

#include "PixelBuffer.hh"

static bool CheckGLXVersion(Display* display, int screen) {
	const char *glxversion;

	glxversion = glXGetClientString(display, GLX_VERSION);
	if (strstr(glxversion, "1.3") == NULL && strstr(glxversion, "1.4") == NULL)
		return false;

	glxversion = glXQueryServerString(display, screen, GLX_VERSION);
	if (strstr(glxversion, "1.3") == NULL && strstr(glxversion, "1.4") == NULL)
		return false;

	return true;
}

PBuffer::PBuffer(int width, int height, int samples) : width_(width), height_(height), display_(NULL), context_(NULL), pbuffer_(None) {
	if ((display_ = XOpenDisplay(NULL)) == NULL)
		throw PBufferException() << "cannot open default X display";

	int screen = DefaultScreen(display_);

	if (!CheckGLXVersion(display_, screen)) {
		XCloseDisplay(display_);
		throw PBufferException() << "GLX 1.3 or 1.4 required, but not available";
	}

	int fbattribs[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
		None, None,
		None, None,
		None
	};

	if (samples > 1) {
		fbattribs[12] = GLX_SAMPLE_BUFFERS;
		fbattribs[13] = 1;
		fbattribs[14] = GLX_SAMPLES;
		fbattribs[15] = samples;
	}

	int pbattribs[] = {
		GLX_PBUFFER_WIDTH, width_,
		GLX_PBUFFER_HEIGHT, height_,
		GLX_LARGEST_PBUFFER, False,
		GLX_PRESERVED_CONTENTS, False,
		None
	};

	int nconfigs;
	GLXFBConfig *fbconfigs;
	if ((fbconfigs = glXChooseFBConfig(display_, screen, fbattribs, &nconfigs)) == NULL) {
		XCloseDisplay(display_);
		throw PBufferException() << "glxChooseFBConfig failed";
	}

	if (nconfigs == 0) {
		XFree(fbconfigs);
		XCloseDisplay(display_);
		throw PBufferException() << "no suitable configs returned by glxChooseFBConfig";
	}

	/* Just use first config */
	GLXFBConfig fbconfig = fbconfigs[0];
	XFree(fbconfigs);

	if ((pbuffer_ = glXCreatePbuffer(display_, fbconfig, pbattribs)) == None) {
		XCloseDisplay(display_);
		throw PBufferException() << "glXCreatePbuffer failed";
	}

	if ((context_ = glXCreateNewContext(display_, fbconfig, GLX_RGBA_TYPE, NULL, True)) == NULL) {
		glXDestroyPbuffer(display_, pbuffer_);
		XCloseDisplay(display_);
		throw PBufferException() << "glXCreateNewContext failed";
	}

	if (!glXMakeCurrent(display_, pbuffer_, context_)) {
		glXDestroyContext(display_, context_);
		glXDestroyPbuffer(display_, pbuffer_);
		XCloseDisplay(display_);
		throw PBufferException() << "glXMakeCurrent failed";
	}
}

PBuffer::~PBuffer() {
	if (!glXMakeCurrent(display_, None, NULL))
		warnx("cannot reset GLX context: glXMakeCurrent failed");
	glXDestroyContext(display_, context_);
	glXDestroyPbuffer(display_, pbuffer_);
	XCloseDisplay(display_);
}

bool PBuffer::IsDirect() const {
	return glXIsDirect(display_, context_);
}

void PBuffer::GetPixels(PixelBuffer& buffer, int x, int y) {
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(x, y, buffer.GetWidth(), buffer.GetHeight(), GL_RGB, GL_UNSIGNED_BYTE, buffer.GetData());
}
