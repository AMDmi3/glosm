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

#ifndef TIMER_HH
#define TIMER_HH

#include <sys/time.h>

/**
 * Simple timer class
 */
class Timer {
public:
	/**
	 * Constructs timer and initializes it with current time
	 */
	Timer();

	/**
	 * Updates timer and returns time passed
	 *
	 * @return seconds passed since construction or last Count()
	 *         call
	 */
	float Count();

protected:
	struct timeval last_;
};

#endif
