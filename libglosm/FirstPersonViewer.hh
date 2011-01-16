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

#ifndef FIRSTPERSONVIEWER_HH
#define FIRSTPERSONVIEWER_HH

#include "Viewer.hh"

class Projection;

class FirstPersonViewer : public Viewer {
public:
	enum Direction {
		FORWARD = 0x0001,
		BACKWARD = 0x0002,
		LEFT = 0x0004,
		RIGHT = 0x0008,
		UP = 0x0010,
		DOWN = 0x0020,
		LOWER = 0x0040,
		HIGHER = 0x0080,

		FAST = 0x1000, // Does not really belong here, but for now..
		SLOW = 0x2000, // Does not really belong here, but for now..
	};

public:
	FirstPersonViewer();
	FirstPersonViewer(const Vector3i& pos);
	FirstPersonViewer(const Vector3i& pos, float yaw, float pitch);

	virtual void SetupViewerMatrix(const Projection& projection) const;
	virtual Vector3i GetPos(const Projection& projection) const;

	void SetFov(float fov);
	void SetAspect(float aspect);

	void SetPos(Vector3i pos);

	Vector3f GetDirection() const;

	void Move(int flags, float speed, float time);
	void HardRotate(float yawdelta, float pitchdelta);

	Vector3d& MutablePos();

protected:
	/* position is actually Vector3i, however that won't allow
	 * smooth movement (e.g. by 1mm 100 times per second, so
	 * just use Vector3d here, which will just transparently
	 * translate from/to Vector3i */
	Vector3d pos_;

	float yaw_;
	float pitch_;

	float fov_;
	float aspect_;
};

#endif
