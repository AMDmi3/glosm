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

/*
 * This test checks whether types of enough length are used for
 * vector operations.
 *
 * Thus, when coordinates are stored as 32-bit values, 64 bits
 * should be used in calculations, as 32 bits are not enough to
 * hold the result of addition/substraction and especially
 * multiplication of two 32 bit values.
 */

#include <limits>
#include <iostream>

#include <glosm/Math.hh>

int main() {
	Vector2i testi(1800000000, 1800000000);
	Vector2l testl(1800000000, 1800000000);

	std::cerr << "sizeof(osmint_t) = " << sizeof(osmint_t) << ", sizeof(osmlong_t) = " << sizeof(osmlong_t) << std::endl;
	std::cerr << "Vector2i(" << testi.x << "," << testi.y << ").LengthSquare() = " << testi.LengthSquare() << std::endl;
	std::cerr << "Vector2l(" << testl.x << "," << testl.y << ").LengthSquare() = " << testl.LengthSquare() << std::endl;

	int result = 0;

	result |= !(testi.LengthSquare() == 6480000000000000000LL);
	result |= !(testl.LengthSquare() == 6480000000000000000LL);

	return result;
}
