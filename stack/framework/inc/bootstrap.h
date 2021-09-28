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

/*! \file bootstrap.h
 * \addtogroup bootstrap
 * \ingroup framework
 * @{
 * \brief Bootstrapping functions for the framework
 *
 */
#ifndef __BOOTSTRAP_H_
#define __BOOTSTRAP_H_
#include "link_c.h"
#include "scheduler.h"


/*! \brief callback from the HAL to initialise the framework
 *
 * This function is not a 'true' task in the sense that it is not registered with the 
 * scheduler. As a result it will only be executed once. 
 *
 * It should be noted that this function is responsible for initialising the scheduler.
 * This is NOT done by the HAL, since the scheduler is part of the framework.
 *
 */
__LINK_C void __framework_bootstrap();

/*! \brief The task scheduled to 'bootstrap' the application.
 *
 * The 'bootstrap' must be provided by the application and is called once the
 * HAL, Framework and included modules have been initialised.
 *
 * This task is only scheduled once, it is the responsibility of the application
 * to reschedule this task or schedule additional tasks if needed
 *
 */
__LINK_C void bootstrap();

#endif

/** @}*/
