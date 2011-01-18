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

#ifndef ORTHOVIEWER_HH
#define ORTHOVIEWER_HH

#include <glosm/Viewer.hh>

/**
 * Viewer describing orthographic projection.
 *
 * This viewer is useful for tile rendering.
 */
class OrthoViewer : public Viewer {
public:
	OrthoViewer();

	virtual void SetupViewerMatrix(const Projection& projection) const;
	virtual Vector3i GetPos(const Projection& projection) const;

	/**
	 * Sets bounding box of a viewport in global coordinades
	 */
	void SetBBox(const BBoxi& pos);

	/**
	 * Sets bounding box for specific tile in mapnik format.
	 *
	 * @param nx X coordinate of a desired tile
	 * @param ny Y coordinate of a desired tile
	 * @param zoom zoom level of a desired tile
	 */
	void SetBBoxForTile(int nx, int ny, int zoom);

	/**
	 * Sets skew for pseudo-3D effect.
	 *
	 * To retain tile proportions, we can't do proper 3D by
	 * changing viewing angle, but we can 'skew' objects that
	 * have height so their sides appear on a top-down view.
	 * Skew factor may be described as a tangent of desired
	 * viewing angle, 1.0 is equal to 45 degrees.
	 *
	 * @param skew skew ratio
	 */
	void SetSkew(float skew);

protected:
	BBoxi bbox_;
	float skew_;
};

#endif
