/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#define PLATFORM_WIZZIMOTE


#ifdef PLATFORM_WIZZIMOTE
#include "wizzimote.h"
#elif defined PLATFORM_ARTESIS
#include "artesis.h"
#elif defined PLATFORM_AGAIDI
#include "agaidi.h"
#else
	#error No platform set
#endif


#endif /* PLATFORM_H_ */
