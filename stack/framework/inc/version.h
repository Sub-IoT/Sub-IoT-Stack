/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
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
 */

/*!
 * \file version.h
 * \addtogroup version
 * \ingroup framework
 * @{
 * \brief The application name and git revision used for building.
 * \author glenn.ergeerts@uantwerpen.be
 */

#ifndef VERSION_H
#define VERSION_H

extern const char _GIT_SHA1[];
extern const char _APP_NAME[];

#define VERSION _GIT_SHA1

#endif // VERSION_H

/** @}*/
