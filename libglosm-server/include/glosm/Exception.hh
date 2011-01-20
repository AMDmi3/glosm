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

#ifndef EXCEPTION_HH
#define EXCEPTION_HH

#include <exception>
#include <string>
#include <sstream>

/**
 * Convenient exception class which supports stream-like appending
 *
 * Example:
 * @code throw Exception("foo") << ", also baz and " << 1 << 2 << 3; @endcode
 */
class Exception: public std::exception {
protected:
	mutable std::stringstream message_;

public:
	Exception(): message_(std::ios_base::out | std::ios_base::app) {
	}

	Exception(const Exception& e): std::exception(), message_(e.what(), std::ios_base::out | std::ios_base::app) {
	}

	Exception(const std::string& message): message_(message, std::ios_base::out | std::ios_base::app) {
	}

	virtual ~Exception() throw() {
	}

	virtual const char* what() const throw() {
		return message_.str().c_str();
	}

	template <class T>
	Exception& operator<<(const T& t) {
		message_ << t;
		return *this;
	}
};

#endif
