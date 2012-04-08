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

#include <glosm/Timer.hh>

#include <stddef.h> /* for NULL */

Timer::Timer() {
	Count();
}

float Timer::Count() {
	struct timeval current;
	gettimeofday(&current, NULL);

	float retval = (float)(current.tv_sec - last_.tv_sec) + (float)(current.tv_usec - last_.tv_usec)/1000000.0f;

	last_ = current;

	return retval;
}
