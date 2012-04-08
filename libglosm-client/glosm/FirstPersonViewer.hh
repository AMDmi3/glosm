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

#ifndef FIRSTPERSONVIEWER_HH
#define FIRSTPERSONVIEWER_HH

#include <glosm/Viewer.hh>

class Projection;
class HeightmapDatasource;

/**
 * Viewer describing perspective projection.
 */
class FirstPersonViewer : public Viewer {
public:
	/**
	 * Movement direction flags.
	 *
	 * These are used to describe in which way (relative to
	 * look direction) a viewer should move to
	 *
	 * LOWER/HIGHER values are actually absolute up/down.
	 */
	enum Direction {
		FORWARD = 0x0001,
		BACKWARD = 0x0002,
		LEFT = 0x0004,
		RIGHT = 0x0008,
		UP = 0x0010,
		DOWN = 0x0020,
		LOWER = 0x0040,
		HIGHER = 0x0080,
	};

protected:
	Vector3f GetDirection() const;
	void FixRotation();
	void FixPosition();

public:
	FirstPersonViewer();
	FirstPersonViewer(const Vector3i& pos);
	FirstPersonViewer(const Vector3i& pos, float yaw, float pitch);

	virtual void SetupViewerMatrix(const Projection& projection) const;
	virtual Vector3i GetPos(const Projection& projection) const;

	void SetFov(float fov);
	void SetAspect(float aspect);

	void SetPos(Vector3i pos);

	/**
	 * Move viewer smoothly
	 *
	 * @param flags combination of Direction constants which
	 *              represents direction for viewer to move to
	 * @param speed movement speed in meters/second
	 * @param time time delta for movement in seconds
	 */
	void Move(int flags, float speed, float time);

	void SetRotation(float yaw, float pitch);

	void Rotate(float yawspeed, float pitchspeed, float time);

	float GetPitch() const;
	float GetYaw() const;

	float GetFov() const;
	float GetAspect() const;

	Vector3d& MutablePos();

	void SetHeightmapDatasource(const HeightmapDatasource* heightmap);

protected:
	const HeightmapDatasource* heightmap_;

protected:
	/** Eye position */
	Vector3d pos_;

	/** Terrain elevation at eye point */
	osmint_t landscape_height_;

	float yaw_;
	float pitch_;

	float fov_;
	float aspect_;
};

#endif
