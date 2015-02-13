/*! \file types.h
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
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 *
 *	\brief Includes all used types

 */

#ifndef __FRM_TYPES_H__
#define __FRM_TYPES_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* \brief the type of all returned errors
 *
 */
typedef int error_t;	//use 'int' since it matches the value expected for errors from <errno.h>
						//and the EFM32GG toolchain actually defines it as such



#endif // __FRM_TYPES_H__
