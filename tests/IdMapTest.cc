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

#include <glosm/id_map.hh>

#include "testing.h"

typedef id_map<unsigned int, unsigned int, sizeof(unsigned int)*8> TestMap;

BEGIN_TEST()
	// create
	TestMap map(8);

	EXPECT_INT(map.size(), 0);
	EXPECT_TRUE(map.empty());
	EXPECT_TRUE(map.begin() == map.end());

	// fill
	int ksum = 0, vsum = 0;
	{
		for (unsigned int i = 0; i < 8; ++i) {
			std::pair<TestMap::iterator, bool> res = map.insert(std::make_pair(i, 1024 - i));
			EXPECT_TRUE(res.second);
			EXPECT_TRUE(res.first->first == i && res.first->second == 1024 - i);
			ksum += i;
			vsum += 1024 - i;
		}

		EXPECT_INT(map.size(), 8);
		EXPECT_TRUE(!map.empty());
	}

	// read
	{
		int testksum = 0, testvsum = 0, iterations = 0;
		for (TestMap::const_iterator i = map.begin(); i != map.end(); ++i, ++iterations) {
			testksum += i->first;
			testvsum += i->second;
		}

		EXPECT_INT(iterations, 8);
		EXPECT_INT(testvsum, vsum);
		EXPECT_INT(testksum, ksum);
	}

	// check miss
	EXPECT_TRUE(map.find(8) == map.end());

	// find
	{
		int testksum = 0, testvsum = 0;
		for (unsigned int i = 0; i < 8; ++i) {
			TestMap::iterator el = map.find(i);
			EXPECT_TRUE(el != map.end());
			assert(el != map.end());
			testksum += el->first;
			testvsum += el->second;
		}

		EXPECT_INT(testvsum, vsum);
		EXPECT_INT(testksum, ksum);
	}

	// fill extra, will trigger rehashes and page allocs
	{
		for (unsigned int i = 8; i < 32; ++i) {
			std::pair<TestMap::iterator, bool> res = map.insert(std::make_pair(i, 1024 - i));
			EXPECT_TRUE(res.second);
			EXPECT_TRUE(res.first->first == i && res.first->second == 1024 - i);
			ksum += i;
			vsum += 1024 - i;
		}

		EXPECT_INT(map.size(), 32);
		EXPECT_TRUE(!map.empty());
	}

	// read
	{
		int testksum = 0, testvsum = 0, iterations = 0;
		for (TestMap::const_iterator i = map.begin(); i != map.end(); ++i, ++iterations) {
			testksum += i->first;
			testvsum += i->second;
		}

		EXPECT_INT(iterations, 32);
		EXPECT_INT(testvsum, vsum);
		EXPECT_INT(testksum, ksum);
	}

	// find
	{
		int testksum = 0, testvsum = 0;
		for (unsigned int i = 0; i < 32; ++i) {
			TestMap::iterator el = map.find(i);
			EXPECT_TRUE(el != map.end());
			assert(el != map.end());
			testksum += el->first;
			testvsum += el->second;
		}

		EXPECT_INT(testvsum, vsum);
		EXPECT_INT(testksum, ksum);
	}

	// pop
	for (unsigned int i = 31; i >= 4; --i) {
		map.erase_last();
		ksum -= i;
		vsum -= 1024 - i;
	}

	// read
	{
		int testksum = 0, testvsum = 0, iterations = 0;
		for (TestMap::const_iterator i = map.begin(); i != map.end(); ++i, ++iterations) {
			testksum += i->first;
			testvsum += i->second;
		}

		EXPECT_INT(iterations, 4);
		EXPECT_INT(testvsum, vsum);
		EXPECT_INT(testksum, ksum);
	}

	// find
	{
		int testksum = 0, testvsum = 0;
		for (unsigned int i = 0; i < 4; ++i) {
			TestMap::iterator el = map.find(i);
			EXPECT_TRUE(el != map.end());
			assert(el != map.end());
			testksum += el->first;
			testvsum += el->second;
		}

		EXPECT_INT(testvsum, vsum);
		EXPECT_INT(testksum, ksum);

		for (unsigned int i = 4; i < 33; ++i) {
			TestMap::iterator el = map.find(i);
			EXPECT_TRUE(el == map.end());
		}
	}

END_TEST()
