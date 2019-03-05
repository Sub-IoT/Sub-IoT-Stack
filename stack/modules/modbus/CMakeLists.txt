# 
# OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
# lowpower wireless sensor communication
#
# Copyright 2017 University of Antwerp
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#Module specific parameters and options can be defined using
#MODULE_OPTION and MODULE_PARAMETER
#See cmake/module_macros.cmake for more information

#Generate the 'module_defs.h'
MODULE_BUILD_SETTINGS_FILE()

#Export the module-specific header files to the application by using
EXPORT_GLOBAL_INCLUDE_DIRECTORIES(.)

#By convention, each module should generate a single 'static' library that can be included by the application
ADD_LIBRARY(modbus STATIC
    HALmodbus/ComL1.c
    HALmodbus/TimersL1.c
    ModulesModbus/Modbus/CRC16.c
    ModulesModbus/Modbus/ModbusRtuSlaveIntrL6-fif.c
    ModulesModbus/Modbus/ModbusRtuSlaveIntrL6.c
    ModulesModbus/Modbus/ModbusRtuSlaveL7.c
    MainApp/TestMode.c
)

ADD_DEFINITIONS("-DUSE_B_L072Z_LRWAN1" "-DSTM32L072xx") # TODO tmp

GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
target_include_directories(modbus PUBLIC
    HALmodbus
    ModulesModbus/Modbus
    ModulesModbus/Common
    MainApp
    ${__global_include_dirs}
    ${CMAKE_BINARY_DIR}/framework/ #framework_defs.h
    ${CMAKE_CURRENT_BINARY_DIR} # MODULE_LORAWAN_defs.h
)

#TARGET_LINK_LIBRARIES(modbus m)
