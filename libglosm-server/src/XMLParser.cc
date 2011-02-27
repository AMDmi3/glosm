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

#include <fcntl.h>
#include <expat.h>

XMLParser::XMLParser() {
}

XMLParser::~XMLParser() {
}

void XMLParser::StartElementWrapper(void* userData, const char* name, const char** atts) {
	static_cast<XMLParser*>(userData)->StartElement(name, atts);
}

void XMLParser::EndElementWrapper(void* userData, const char* name) {
	static_cast<XMLParser*>(userData)->EndElement(name);
}

void XMLParser::Load(const char* filename) {
	int f = 0;
	XML_Parser parser = NULL;

	/* if filename = "-", work with stdin */
	if (strcmp(filename, "-") != 0 && (f = open(filename, O_RDONLY)) == -1)
		throw SystemError() << "cannot open input file";

	/* Create and setup parser */
	if ((parser = XML_ParserCreate(NULL)) == NULL) {
		close(f);
		throw Exception() << "cannot create XML parser";
	}

	XML_SetElementHandler(parser, StartElementWrapper, EndElementWrapper);
	XML_SetUserData(parser, this);

	/* Parse file */
	try {
		char buf[65536];
		ssize_t len;
		do {
			if ((len = read(f, buf, sizeof(buf))) < 0)
				throw SystemError() << "input read error";
			if (XML_Parse(parser, buf, len, len == 0) == XML_STATUS_ERROR)
				throw ParsingException() << XML_ErrorString(XML_GetErrorCode(parser));
		} while (len != 0);
	} catch (ParsingException &e) {
		ParsingException verbose;
		verbose << "input parsing error: " << e.what() << " at line " << XML_GetCurrentLineNumber(parser) << " pos " << XML_GetCurrentColumnNumber(parser);
		close(f);
		XML_ParserFree(parser);
		throw verbose;
	} catch (...) {
		close(f);
		XML_ParserFree(parser);
		throw;
	}

	XML_ParserFree(parser);
	close(f);
}
