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

#include <glosm/ParsingHelpers.hh>

int ParseCoord(const char* str) {
	return ParseInt<7>(str);
}

int ParseEle(const char* str) {
	return ParseInt<2>(str);
}

BBoxi ParseBounds(const char** atts) {
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

BBoxi ParseBound(const char** atts) {
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
