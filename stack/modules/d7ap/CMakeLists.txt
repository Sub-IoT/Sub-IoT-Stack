#[[
Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.

This file is part of Sub-IoT.
See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

# Module specific parameters and options can be defined using
# MODULE_OPTION and MODULE_PARAMETER
# See cmake/module_macros.cmake for more information

INCLUDE_DIRECTORIES(inc)

MODULE_PARAM(${MODULE_PREFIX}_MAX_CLIENT_COUNT "2" STRING "The maximum number of d7ap clients")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_MAX_CLIENT_COUNT)

MODULE_PARAM(${MODULE_PREFIX}_MAX_REQUEST_COUNT "2" STRING "The maximum number of requests per session")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_MAX_REQUEST_COUNT)

MODULE_PARAM(${MODULE_PREFIX}_MAX_SESSION_COUNT "2" STRING "The maximum number of session")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_MAX_SESSION_COUNT)

MODULE_PARAM(${MODULE_PREFIX}_PACKET_QUEUE_SIZE "3" STRING "The max number of packets which can be used concurrently")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_PACKET_QUEUE_SIZE)

MODULE_PARAM(${MODULE_PREFIX}_FIFO_COMMAND_BUFFER_SIZE "200" STRING "The D7ASP FIFO command buffer size")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_FIFO_COMMAND_BUFFER_SIZE)

MODULE_PARAM(${MODULE_PREFIX}_FIFO_MAX_REQUESTS_COUNT "2" STRING "The maximum number of requests in a D7ASP FIFO (before flush terminates)")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_FIFO_MAX_REQUESTS_COUNT)

MODULE_PARAM(${MODULE_PREFIX}_RAW_PACKET_SIZE "512" STRING "The maximum number of bytes that can be received or transmitted. Should be a multiple of 4")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_RAW_PACKET_SIZE)

MODULE_PARAM(${MODULE_PREFIX}_PAYLOAD_SIZE "239" STRING "The maximum data size in bytes that can be recieved or transmitted")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_PAYLOAD_SIZE)

MODULE_OPTION(${MODULE_PREFIX}_NLS_ENABLED "Enable Security in NETW layer" TRUE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_NLS_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_PHY_LOG_ENABLED "Enable logging for PHY layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_PHY_LOG_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_DLL_LOG_ENABLED "Enable logging for DLL layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_DLL_LOG_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_FWK_LOG_ENABLED "Enable logging for FWK" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_FWK_LOG_ENABLED)

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

MODULE_OPTION(${MODULE_PREFIX}_EM_ENABLED "Enable engineering mode" TRUE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_EM_ENABLED)

MODULE_OPTION(${MODULE_PREFIX}_EM_LOG_ENABLED "Enable logging for the engineering mode" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_EM_LOG_ENABLED)

#Generate the 'module_defs.h'
MODULE_BUILD_SETTINGS_FILE()

#Export the module-specific header files to the application by using
EXPORT_GLOBAL_INCLUDE_DIRECTORIES(.)

#By convention, each module should generate a single 'static' library that can be included by the application
ADD_LIBRARY(d7ap STATIC
    d7ap_stack.c
    d7ap.c
    d7asp.c
    d7atp.c
    d7anp.c
    engineering_mode.c
    packet_queue.c
    packet.c
    dll.c
    phy.c
)

GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
target_include_directories(d7ap PUBLIC
	${__global_include_dirs}
	${CMAKE_CURRENT_BINARY_DIR} # MODULE_D7AP_defs.h
    inc
)

TARGET_LINK_LIBRARIES(d7ap PRIVATE m d7ap_fs)
