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

MODULE_OPTION(${MODULE_PREFIX}_LOG_ENABLED "Enable logging for ALP layer" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_LOG_ENABLED)

MODULE_PARAM(${MODULE_PREFIX}_MAX_ACTIVE_COMMAND_COUNT "8" STRING "The maximum number of active ALP commands")
MODULE_HEADER_DEFINE(NUMBER ${MODULE_PREFIX}_MAX_ACTIVE_COMMAND_COUNT)

MODULE_OPTION(${MODULE_PREFIX}_LOCK_KEY_FILES "Lock the filesystem permissions of the root and user keys to not be read- and writeable" TRUE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_LOCK_KEY_FILES)

MODULE_OPTION(${MODULE_PREFIX}_USE_EXTERNAL_MODEM "Activate interface to external modem" FALSE)
MODULE_HEADER_DEFINE(BOOL ${MODULE_PREFIX}_USE_EXTERNAL_MODEM)

# Generate the 'module_defs.h'
MODULE_BUILD_SETTINGS_FILE()

# Export the module-specific header files to the application by using
EXPORT_GLOBAL_INCLUDE_DIRECTORIES(.)

SET(sources
  alp_layer.c
  alp.c
)

if(FRAMEWORK_MODEM_INTERFACE_ENABLED)
  list(APPEND sources serial_interface.c)
endif()

if(MODULE_D7AP)
  list(APPEND sources d7ap_interface.c)
endif()

if(MODULE_LORAWAN)
  list(APPEND sources lorawan_interface.c)
endif()

if(MODULE_ALP_USE_EXTERNAL_MODEM)
  list(APPEND sources modem.c)
endif()

  # By convention, each module should generate a single 'static' library that can be included by the application
ADD_LIBRARY(alp STATIC
  ${sources}
)

GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
target_include_directories(alp PUBLIC
  ${__global_include_dirs}
  ${CMAKE_BINARY_DIR}/framework/ # framework_defs.h
  ${CMAKE_CURRENT_BINARY_DIR} # MODULE_D7AP_defs.h
  inc
)

GET_PROPERTY(__global_compile_definitions GLOBAL PROPERTY GLOBAL_COMPILE_DEFINITIONS)
TARGET_COMPILE_DEFINITIONS(alp PUBLIC ${__global_compile_definitions})

TARGET_LINK_LIBRARIES(alp PRIVATE m)

if(MODULE_D7AP_FS)
  TARGET_LINK_LIBRARIES(alp PUBLIC d7ap_fs)
endif()

if(MODULE_D7AP)
  TARGET_LINK_LIBRARIES(alp PUBLIC d7ap)
endif()
