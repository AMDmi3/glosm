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

#include "Viewer.hh"

class OrthoViewer : public Viewer {
public:
	OrthoViewer();

	virtual void SetupViewerMatrix(const Projection& projection) const;
	virtual Vector3i GetPos(const Projection& projection) const;

	void SetBBox(const BBoxi& pos);
	void SetBBoxForTile(int nx, int ny, int zoom);

	void SetSkew(float skew);

protected:
	BBoxi bbox_;
	float skew_;
};

#endif
