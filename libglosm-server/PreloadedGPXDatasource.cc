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

#include <glosm/PreloadedGPXDatasource.hh>
#include <glosm/ParsingHelpers.hh>

PreloadedGPXDatasource::PreloadedGPXDatasource() : XMLParser(XMLParser::HANDLE_ELEMENTS | XMLParser::HANDLE_CHARDATA) {
}

PreloadedGPXDatasource::~PreloadedGPXDatasource() {
}

void PreloadedGPXDatasource::StartElement(const char* name, const char** atts) {
	if (tag_level_ == 4 && current_tag_ == TRKPT && StrEq<-1>(name, "ele")) {
		current_tag_ = ELE;
	} else if (tag_level_ == 3 && current_tag_ == TRKSEG && StrEq<-1>(name, "trkpt")) {
		current_tag_ = TRKPT;

		int lat = 0;
		int lon = 0;
		for (const char** att = atts; *att; ++att) {
			if (StrEq<2>(*att, "lat"))
				lat = ParseCoord(*(++att));
			else if (StrEq<2>(*att, "lon"))
				lon = ParseCoord(*(++att));
			else
				++att;
		}

		points_.push_back(Vector3i(lon, lat, 0));
	} else if (tag_level_ == 2 && current_tag_ == TRK && StrEq<-1>(name, "trkseg")) {
		current_tag_ = TRKSEG;
	} else if (tag_level_ == 1 && current_tag_ == GPX && StrEq<-1>(name, "trk")) {
		current_tag_ = TRK;
	} else if (tag_level_ == 0 && current_tag_ == NONE && StrEq<-1>(name, "gpx")) {
		current_tag_ = GPX;
	} else if (tag_level_ == 0) {
		throw ParsingException() << "unexpected root element (" << name << " instead of gpx)";
	}

	++tag_level_;
}

void PreloadedGPXDatasource::EndElement(const char* /*name*/) {
	if (tag_level_ == 5 && current_tag_ == ELE)
		current_tag_ = TRKPT;
	else if (tag_level_ == 4 && current_tag_ == TRKPT)
		current_tag_ = TRKSEG;
	else if (tag_level_ == 3 && current_tag_ == TRKSEG)
		current_tag_ = TRK;
	else if (tag_level_ == 2 && current_tag_ == TRK)
		current_tag_ = GPX;
	else if (tag_level_ == 1 && current_tag_ == GPX)
		current_tag_ = NONE;

	--tag_level_;
}

void PreloadedGPXDatasource::CharacterData(const char* data, int len) {
	std::string buf(data, len);
	if (tag_level_ == 5 && current_tag_ == ELE)
		points_.back().z = ParseEle(buf.c_str());
}

void PreloadedGPXDatasource::Load(const char* filename) {
	current_tag_ = NONE;
	tag_level_ = 0;

	XMLParser::Load(filename);
}

void PreloadedGPXDatasource::GetPoints(std::vector<Vector3i>& out, const BBoxi& bbox) const {
	for (PointsVector::const_iterator i = points_.begin(); i != points_.end(); ++i)
		if (bbox.Contains(*i))
			out.push_back(*i);
}
