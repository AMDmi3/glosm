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

/*
 * This test checks exception functionality.
 */

#include <glosm/Exception.hh>

#include <iostream>
#include <cstring>
#include <cerrno>

class MyException : public Exception {
};

int main() {
	int ret = 0;

	/* Test generic stream-like behaviour */
	try {
		throw Exception() << "foo " << 123 << " bar";
	} catch (Exception& e) {
		std::cerr << "Cought right exception: " << e.what() << std::endl;
		if (strcmp(e.what(), "foo 123 bar") != 0) {
			std::cerr << "But the message is wrong" << std::endl;
			ret = 1;
		}
	} catch (...) {
		std::cerr << "Cought wrong exception" << std::endl;
		ret = 1;
	}

	/* Test proper inheritance: return type of operator<<
	 * should be the save as original exception */
	try {
		throw MyException();
	} catch (MyException& e) {
		std::cerr << "Cought right exception: " << std::endl;
	} catch (...) {
		std::cerr << "Cought wrong exception" << std::endl;
		ret = 1;
	}

	/* Test system errors. Message should be appended */
	try {
		errno = EINVAL;
		throw SystemError() << "TEST" << 123;
	} catch (std::exception& e) {
		std::cerr << "Cought right exception: " << e.what() << std::endl;
		if (strstr(e.what(), "TEST123") == NULL) {
			std::cerr << "But user message was lost" << std::endl;
			ret = 1;
		}
		if (strstr(e.what(), strerror(errno)) == NULL) {
			std::cerr << "But system message was lost" << std::endl;
			ret = 1;
		}
	} catch (...) {
		std::cerr << "Cought wrong exception" << std::endl;
		ret = 1;
	}

	/* Test that special exception (system error) may be cast to
	 * generic one without loosing any data */
	try {
		errno = EINVAL;
		throw Exception(SystemError() << "TEST" << 123);
	} catch (std::exception& e) {
		std::cerr << "Cought right exception: " << e.what() << std::endl;
		if (strstr(e.what(), "TEST123") == NULL) {
			std::cerr << "But user message was lost" << std::endl;
			ret = 1;
		}
		if (strstr(e.what(), strerror(errno)) == NULL) {
			std::cerr << "But system message was lost" << std::endl;
			ret = 1;
		}
	} catch (...) {
		std::cerr << "Cought wrong exception" << std::endl;
		ret = 1;
	}

	return ret;
}
