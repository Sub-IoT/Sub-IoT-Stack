# 
# OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
# lowpower wireless sensor communication
#
# Copyright 2015 University of Antwerp
# Copyright 2018 CORTUS S.A
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


MODULE_OPTION(${MODULE_PREFIX}_LOG_ENABLED "Enable logging for ALP layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_LOG_ENABLED)

MODULE_PARAM(${MODULE_PREFIX}_MAX_ACTIVE_COMMAND_COUNT "3" STRING "The maximum number of active ALP commands")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_MAX_ACTIVE_COMMAND_COUNT)

#Generate the 'module_defs.h'
MODULE_BUILD_SETTINGS_FILE()

#Export the module-specific header files to the application by using
EXPORT_GLOBAL_INCLUDE_DIRECTORIES(.)

#By convention, each module should generate a single 'static' library that can be included by the application
ADD_LIBRARY(alp STATIC
    serial_interface.c
    alp_layer.c
    alp.c
)

GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
target_include_directories(alp PUBLIC
    ${__global_include_dirs}
    ${CMAKE_BINARY_DIR}/framework/ #framework_defs.h
    ${CMAKE_CURRENT_BINARY_DIR} # MODULE_D7AP_defs.h
)

GET_PROPERTY(__global_compile_definitions GLOBAL PROPERTY GLOBAL_COMPILE_DEFINITIONS)
TARGET_COMPILE_DEFINITIONS(alp PUBLIC ${__global_compile_definitions})

TARGET_LINK_LIBRARIES(alp m)
