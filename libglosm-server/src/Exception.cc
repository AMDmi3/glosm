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

#include <glosm/Exception.hh>

#include <errno.h>
#include <stdio.h>

namespace Private {
	Exception::Exception(): message_(std::ios_base::out | std::ios_base::app) {
	}

	Exception::Exception(const Exception& e): std::exception(), message_(e.message_.str(), std::ios_base::out | std::ios_base::app) {
	}

	Exception::~Exception() throw() {
	}

	const char* Exception::what() const throw() {
		return message_.str().c_str();
	}
};

SystemError::SystemError(): full_message_(std::ios_base::out | std::ios_base::app), errno_(errno) {
}

SystemError::SystemError(const SystemError& e): Exception(e), full_message_(std::ios_base::out | std::ios_base::app), errno_(e.errno_) {
}

SystemError::~SystemError() throw() {
}

const char* SystemError::what() const throw() {
	full_message_.clear();
	full_message_ << message_.str() << ": " << strerror(errno_);
	return full_message_.str().c_str();
}
