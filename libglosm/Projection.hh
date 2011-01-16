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

#ifndef PROJECTION_HH
#define PROJECTION_HH

#include "Math.hh"

#include <vector>

class Projection {
private:
	typedef Vector3f(*ProjectFunction)(const Vector3i&, const Vector3i&);
	typedef Vector3i(*UnProjectFunction)(const Vector3f&, const Vector3i&);

private:
	ProjectFunction project_;
	UnProjectFunction unproject_;

protected:
	Projection(ProjectFunction pf, UnProjectFunction uf);

public:
	Vector3f Project(const Vector3i& point, const Vector3i& ref) const;
	Vector3i UnProject(const Vector3f& point, const Vector3i& ref) const;

	void ProjectPoints(const std::vector<Vector3i>& in, const Vector3i& ref, std::vector<Vector3f>& out) const;
};

#endif
