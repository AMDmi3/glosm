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

#include <glosm/DummyHeightmap.hh>

DummyHeightmap::DummyHeightmap(osmint_t height) : height_(height) {
}

DummyHeightmap::~DummyHeightmap() {
}

void DummyHeightmap::GetHeightmap(const BBoxi& bbox, int extramargin, Heightmap& out) const {
	out.width = 2 + extramargin * 2;
	out.height = 2 + extramargin * 2;

	out.bbox = BBoxi(
				bbox.left - (bbox.right - bbox.left) * extramargin,
				bbox.bottom - (bbox.top - bbox.bottom) * extramargin,
				bbox.right + (bbox.right - bbox.left) * extramargin,
				bbox.top + (bbox.top - bbox.bottom) * extramargin
			);

	out.points.resize(out.width * out.height);

	for (Heightmap::HeightmapPoints::iterator i = out.points.begin(); i != out.points.end(); ++i)
		*i = height_;
}

osmint_t DummyHeightmap::GetHeight(const Vector2i& /*unused*/) const {
	return height_;
}
