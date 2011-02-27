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
 * Parses langitude/latitude into fixed-point number
 *
 * Processes geo coordinates in [-]NNN.NNNNNNN format and returns
 * fixed-point representation with 7 numbers after decimal dot
 */
static int ParseCoord(const char* str) {
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
			if (!haddot && value > 180)
				throw ParsingException() << "bad coordinate format (value too large)";
			if (haddot && ++fracdig == 7)
				break;
		} else if (*cur == '.') {
			haddot++;
		} else {
			throw ParsingException() << "bad coordinate format (unexpected symbol)";
		}
	}

	if (haddot > 1)
		throw ParsingException() << "bad coordinate format (multiple dots)";

	for (; fracdig < 7; ++fracdig)
		value *= 10;

	return neg ? -value : value;
}

/**
 * Parses attributes of <bounds> tag
 *
 * This tag is used in both OSM and GPX files to denote bounding
 * box of a dataset
 *
 * @param atts tag attrubutes from XML parser
 * @return resulting bbox
 */
static BBoxi ParseBounds(const char** atts) {
	BBoxi bbox(BBoxi::Empty());

	for (const char** att = atts; *att; ++att) {
		if (StrEq<-1>(*att, "minlat"))
			bbox.bottom = ParseCoord(*(++att));
		else if (StrEq<-1>(*att, "maxlat"))
			bbox.top = ParseCoord(*(++att));
		else if (StrEq<-1>(*att, "minlon"))
			bbox.left = ParseCoord(*(++att));
		else if (StrEq<-1>(*att, "maxlon"))
			bbox.right = ParseCoord(*(++att));
		else
			++att;
	}

	if (bbox.IsEmpty())
		throw ParsingException() << "incorrect bounding box";

	return bbox;
}

/**
 * Parses attributes of <bound> tag
 *
 * This tag is encountered in OSM files produced by osmosis.
 *
 * @param atts tag attrubutes from XML parser
 * @return resulting bbox
 */
static BBoxi ParseBound(const char** atts) {
	BBoxi bbox(BBoxi::Empty());

	for (const char** att = atts; *att; ++att) {
		if (StrEq<-1>(*att, "box")) {
			std::string s(*(++att));
			/* comma positions */
			size_t cpos1, cpos2, cpos3;
			if ((cpos1 = s.find(',')) == std::string::npos)
				throw ParsingException() << "bad bbox format";
			if ((cpos2 = s.find(',', cpos1+1)) == std::string::npos)
				throw ParsingException() << "bad bbox format";
			if ((cpos3 = s.find(',', cpos2+1)) == std::string::npos)
				throw ParsingException() << "bad bbox format";

			bbox.bottom = ParseCoord(s.substr(0, cpos1).c_str());
			bbox.left = ParseCoord(s.substr(cpos1+1, cpos2-cpos1-1).c_str());
			bbox.top = ParseCoord(s.substr(cpos2+1, cpos3-cpos2-1).c_str());
			bbox.right = ParseCoord(s.substr(cpos3+1).c_str());
		} else {
			++att;
		}
	}

	return bbox;
}
