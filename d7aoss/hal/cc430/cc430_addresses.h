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
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef ADDRESSES_H
#define ADDRESSES_H

#include <msp430.h>

#ifndef __MSP430_BASEADDRESS_PORT1_R__
#define __MSP430_BASEADDRESS_PORT1_R__ PAIN_
#endif

#ifndef __MSP430_BASEADDRESS_PORT2_R__
#define __MSP430_BASEADDRESS_PORT2_R__ PAIN_
#endif

#ifndef __MSP430_BASEADDRESS_PORT3_R__
#define __MSP430_BASEADDRESS_PORT3_R__ PBIN_
#endif

#ifndef __MSP430_BASEADDRESS_RTC__
#define __MSP430_BASEADDRESS_RTC__ RTCCTL01_L
#endif
#endif // ADDRESSES_H
