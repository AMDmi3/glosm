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

#include <cerrno>

namespace Private {
	void SafeStringBuffer::EnsureSize(unsigned int size) {
		unsigned int newallocated_ = allocated_;
		while (size > newallocated_)
			newallocated_ *= 2;

		if (newallocated_ > allocated_) {
			char* newbuffer = new char[newallocated_];
			memcpy(newbuffer, buffer_, used_);
			delete[] buffer_;
			buffer_ = newbuffer;
			allocated_ = newallocated_;
		}
	}

	SafeStringBuffer::SafeStringBuffer(unsigned int reserve)
		: reserve_(reserve),
		  allocated_(initial_size_ + reserve),
		  used_(0),
		  buffer_(new char[allocated_]) {
	}

	SafeStringBuffer::SafeStringBuffer(const char* message)
		: reserve_(0),
		  allocated_(strlen(message) + 1),
		  used_(allocated_ - 1),
		  buffer_(new char[allocated_]) {
		strcpy(buffer_, message);
	}

	SafeStringBuffer::SafeStringBuffer(const SafeStringBuffer& s)
		: std::streambuf(),
		  reserve_(s.reserve_),
		  allocated_(s.allocated_),
		  used_(s.used_),
		  buffer_( new char[allocated_]) {
		memcpy(buffer_, s.buffer_, used_);
	}

	SafeStringBuffer::~SafeStringBuffer() {
		delete[] buffer_;
	}

	void SafeStringBuffer::SetReserve(unsigned int reserve) {
		EnsureSize(used_ + reserve + 1);
		reserve_ = reserve;
	}

	std::streamsize SafeStringBuffer::xsputn(const char* s, std::streamsize n) {
		EnsureSize(used_ + n + reserve_ + 1);
		memcpy(buffer_ + used_, s, n);
		used_ += n;
		return n;
	}

	std::streamsize SafeStringBuffer::AppendReserve(const char* s, std::streamsize n) throw() {
		if (n > reserve_)
			n = reserve_;

		memcpy(buffer_ + used_, s, n);
		used_ += n;
		reserve_ -= n;

		return n;
	}

	const char* SafeStringBuffer::c_str() throw() {
		buffer_[used_] = '\0';
		return buffer_;
	}

	ExceptionBase::ExceptionBase() {
	}

	ExceptionBase::ExceptionBase(const ExceptionBase& e): std::exception(), message_(e.what()/*message_*/) {
	}

	ExceptionBase::~ExceptionBase() throw() {
	}

	const char* ExceptionBase::what() const throw() {
		return message_.c_str();
	}
};

SystemError::SystemError() : errno_(errno) {
	message_.SetReserve(strlen(strerror(errno)) + 3);
}

SystemError::SystemError(const SystemError& e): Exception(e), errno_(e.errno_) {
}

SystemError::~SystemError() throw() {
}

const char* SystemError::what() const throw() {
	message_.AppendReserve(" (", 2);
	message_.AppendReserve(strerror(errno), strlen(strerror(errno)));
	message_.AppendReserve(")", 1);
	return message_.c_str();
}
