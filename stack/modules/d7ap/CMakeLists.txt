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

MODULE_PARAM(${MODULE_PREFIX}_MAX_CLIENT_COUNT "4" STRING "The maximum number of d7ap clients")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_MAX_CLIENT_COUNT)

MODULE_PARAM(${MODULE_PREFIX}_MAX_REQUEST_COUNT "5" STRING "The maximum number of requests per session")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_MAX_REQUEST_COUNT)

MODULE_PARAM(${MODULE_PREFIX}_MAX_SESSION_COUNT "5" STRING "The maximum number of session")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_MAX_SESSION_COUNT)

MODULE_PARAM(${MODULE_PREFIX}_PACKET_QUEUE_SIZE "5" STRING "The max number of packets which can be used concurrently")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_PACKET_QUEUE_SIZE)

MODULE_PARAM(${MODULE_PREFIX}_FIFO_COMMAND_BUFFER_SIZE "100" STRING "The D7ASP FIFO command buffer size")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_FIFO_COMMAND_BUFFER_SIZE)

MODULE_PARAM(${MODULE_PREFIX}_FIFO_MAX_REQUESTS_COUNT "8" STRING "The maximum number of requests in a D7ASP FIFO (before flush terminates)")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_FIFO_MAX_REQUESTS_COUNT)

MODULE_OPTION(${MODULE_PREFIX}_NLS_ENABLED "Enable Security in NETW layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_NLS_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_DLL_LOG_ENABLED "Enable logging for DLL layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_DLL_LOG_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_NP_LOG_ENABLED "Enable logging for NETW layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_NP_LOG_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_TP_LOG_ENABLED "Enable logging for TRANS layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_TP_LOG_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_SP_LOG_ENABLED "Enable logging for SESSION layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_SP_LOG_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_LOG_ENABLED "Enable logging for the d7a top layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_LOG_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_PACKET_LOG_ENABLED "Enable logging for misc logs (not directly belonging to a specificlayer)" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_PACKET_LOG_ENABLED)

#Generate the 'module_defs.h'
MODULE_BUILD_SETTINGS_FILE()

#Export the module-specific header files to the application by using
EXPORT_GLOBAL_INCLUDE_DIRECTORIES(.)

#By convention, each module should generate a single 'static' library that can be included by the application
ADD_LIBRARY(d7ap STATIC
    d7ap_stack.c
    d7ap.c
    d7ap_fs.c
    d7ap_fs_data.c
    d7asp.c
    d7atp.c
    d7anp.c
    packet_queue.c
    packet.c
    dll.c
)

GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
target_include_directories(d7ap PUBLIC
	${__global_include_dirs}
	${CMAKE_BINARY_DIR}/framework/ #framework_defs.h
	${CMAKE_CURRENT_BINARY_DIR} # MODULE_D7AP_defs.h
)

TARGET_LINK_LIBRARIES(d7ap m)
