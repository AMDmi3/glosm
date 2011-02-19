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

#ifndef UTIL_GLEXT_HH
#define UTIL_GLEXT_HH

#if defined(WITH_GLES)
#	include <GLES/glext.h>
#elif defined(WITH_GLES2)
#	include <GLES2/gl2ext.h>
#elif defined(__APPLE__)
#	include <OpenGL/glext.h>
#else
#	include <GL/glext.h>
#endif

#endif
