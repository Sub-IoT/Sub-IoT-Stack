/*!
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
