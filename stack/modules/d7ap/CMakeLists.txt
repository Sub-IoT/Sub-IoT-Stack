# 
# OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
# lowpower wireless sensor communication
#
# Copyright 2015 University of Antwerp
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

#Export the module-specific header files to the application by using
GLOBAL_INCLUDE_DIRECTORIES(dll .) # TODO we export lower layers now during development, later only upper should be exported

ADD_SUBDIRECTORY(dll)

#By convention, each module should generate a single 'static' library that can be included by the application
ADD_LIBRARY(d7ap STATIC
    d7ap_stack.c
    alp.c
    d7asp.c
    trans.c
    fs.c
    packet_queue.c
    packet.c
    crc.c
    $<TARGET_OBJECTS:d7ap_dll>
)
