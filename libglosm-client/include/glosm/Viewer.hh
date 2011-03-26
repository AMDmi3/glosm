/*
 * Copyright (C) 2010-2011 Dmitry Marakasov
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

#ifndef VIEWER_HH
#define VIEWER_HH

#include <glosm/Math.hh>

class Projection;

/**
 * Abstract base class for all viewers.
 *
 * Viewer is a way to represent which part of map we need to render.
 * For example, for first-person viewer that may be eye's position
 * and look direction. This class is used in rendering process to
 * properly setup projection matrix and in layers to determine which
 * objects are close enough to eye to be loaded and displayed.
 */
class Viewer {
public:
	/**
	 * Setups OpenGL projection matrix for the viewer
	 *
	 * @param projection projection used in this world
	 */
	virtual void SetupViewerMatrix(const Projection& projection) const = 0;

	/**
	 * Returns pseudo-position of a viewer.
	 *
	 * @param projection projection used in the world
	 * @return pseudo-position of a viewer
	 */
	virtual Vector3i GetPos(const Projection& projection) const = 0;
};

#endif
