/*!
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *
 * 	Select the correct platform in d7ass.h
 *
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "../../../d7aoss.h"


#ifdef PLATFORM_WIZZIMOTE
#include "wizzimote.h"
#elif defined PLATFORM_ARTESIS
#include "artesis.h"
#elif defined PLATFORM_AGAIDI
#include "agaidi.h"
#elif defined PLATFORM_CHRONOS
#include "chronos.h"
#elif defined PLATFORM_BELIGUM
#include "beligum.h"
#elif defined PLATFORM_MATRIX
#include "matrix_tp1104r3.h"
#else
	#error No platform set
#endif


#endif /* PLATFORM_H_ */
