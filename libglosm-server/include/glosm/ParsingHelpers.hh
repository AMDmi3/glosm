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

#ifndef PARSINGHELPERS_HH
#define PARSINGHELPERS_HH

#include <glosm/XMLParser.hh>
#include <glosm/BBox.hh>

#include <cstring>

#ifdef TRUSTED_XML
/* this is a shortcut for string comparison used where we know
 * that the string e.g. bay only be 'way', 'node', or 'relation'
 * in which case we can only check first letter - this lends
 * ~10% performance gain */
template <int I>
inline bool StrEq(const char* one, const char* two) {
	return strncmp(one, two, I) == 0;
}

template<>
inline bool StrEq<-1>(const char* one, const char* two) {
	return strcmp(one, two) == 0;
}

template<>
inline bool StrEq<0>(const char* one, const char* two) {
	return true;
}

template<>
inline bool StrEq<1>(const char* one, const char* two) {
	return one[0] == two[0];
}

template<>
inline bool StrEq<2>(const char* one, const char* two) {
	return one[0] == two[0] && one[0] != '\0' && one[1] == two[1];
}
#else
template <int I>
inline bool StrEq(const char* one, const char* two) {
	return strcmp(one, two) == 0;
}
#endif

/**
 * Parses decimal floating point number into fixed-point value
 */
template<int I>
static int ParseInt(const char* str) {
	int value = 0;
	int fracdig = 0;
	int haddot = 0;
	bool neg = false;
	const char* cur = str;

	if (*cur == '-') {
		neg = true;
		cur++;
	}

	for (; *cur != '\0'; ++cur) {
		if (*cur >= '0' && *cur <= '9') {
			value = value * 10 + *cur - '0';
			if (haddot && ++fracdig == I)
				break;
		} else if (*cur == '.') {
			haddot++;
		} else {
			throw ParsingException() << "bad coordinate format (unexpected symbol)";
		}
	}

	if (haddot > 1)
		throw ParsingException() << "bad coordinate format (multiple dots)";

	for (; fracdig < I; ++fracdig)
		value *= 10;

	return neg ? -value : value;
}

/**
 * Parses langitude/latitude into fixed-point number
 */
int ParseCoord(const char* str);

/**
 * Parses elevation into fixed-point number
 */
int ParseEle(const char* str);

/**
 * Parses attributes of <bounds> tag
 *
 * This tag is used in both OSM and GPX files to denote bounding
 * box of a dataset
 *
 * @param atts tag attrubutes from XML parser
 * @return resulting bbox
 */
BBoxi ParseBounds(const char** atts);

/**
 * Parses attributes of <bound> tag
 *
 * This tag is encountered in OSM files produced by osmosis.
 *
 * @param atts tag attrubutes from XML parser
 * @return resulting bbox
 */
BBoxi ParseBound(const char** atts);

#endif
