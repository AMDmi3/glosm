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

#include <glosm/Projection.hh>

Projection::Projection(ProjectFunction pf, UnProjectFunction uf): project_(pf), unproject_(uf) {
}

Vector3f Projection::Project(const Vector3i& point, const Vector3i& ref) const {
	return project_(point, ref);
}

Vector3i Projection::UnProject(const Vector3f& point, const Vector3i& ref) const {
	return unproject_(point, ref);
}

void Projection::ProjectPoints(const std::vector<Vector3i>& in, const Vector3i& ref, std::vector<Vector3f>& out) const {
	/* "slow" fallback implementation; though virtual calls
	 * add overhead, it's too small (~3%) to even care; but this
	 * may be redefined in subclasses */
	out.reserve(out.size() + in.size());
	for (std::vector<Vector3i>::const_iterator i = in.begin(); i != in.end(); ++i)
		out.push_back(project_(*i, ref));
}
