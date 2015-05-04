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
