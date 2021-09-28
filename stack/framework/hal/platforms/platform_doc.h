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

/*! \file platform_doc.h
 * \defgroup platform platform
 *
 * \brief A platform defines a combination of an MCU type, an RF chip, optional other chips and the wiring on the board.
 * 	This means there is one platform defined for each type of board.
 *
 * The platform is responsible for initializing the MCU and other chips, bootstrapping the framework and finally start the scheduler loop.
 * For example the pins used for UART, SPI, LEDs etc are defined here. The main() function should be implemented here.
 */