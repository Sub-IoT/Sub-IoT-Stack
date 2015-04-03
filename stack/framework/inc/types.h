/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file types.h
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
