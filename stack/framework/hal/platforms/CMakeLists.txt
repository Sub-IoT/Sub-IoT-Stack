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

SET(PLATFORM_EXTRA_PLATFORMS_DIR "" CACHE STRING "Path to directory containing extra, out of tree platform implementations (optional)")

#Scan all platforms in the current directory for those that are
#compatible with the current toolchain
LIST_SUBDIRS(PLATFORM_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
IF(NOT(PLATFORM_EXTRA_PLATFORMS_DIR STREQUAL ""))
    LIST_SUBDIRS(EXTRA_PLATFORM_DIRS ${PLATFORM_EXTRA_PLATFORMS_DIR})
ENDIF()
LIST(APPEND PLATFORM_DIRS ${EXTRA_PLATFORM_DIRS})
SET(SUPPORTED_PLATFORMS "")
FOREACH(dir ${PLATFORM_DIRS})
    GET_REQUIRED_TOOLCHAIN(${dir} req_tc)
    IF(req_tc STREQUAL "${TOOLCHAIN}")
    GET_FILENAME_COMPONENT(__platform_name "${dir}" NAME)
    LIST(APPEND SUPPORTED_PLATFORMS "${__platform_name}")
    SET(PLATFORM_PATH_${__platform_name} ${dir})
    ENDIF()
ENDFOREACH()
#If no platforms are compatible with the current toolchain, generate an error
IF(NOT SUPPORTED_PLATFORMS)
    MESSAGE(SEND_ERROR "There are no supported platforms for the selected toolchain ${TOOLCHAIN}")
ELSE()
#Otherwise add a parameter to allow the user to select the platform to use
    STRING(REPLACE ";" " " SUPPORTED_PLATFORMS_STRING "${SUPPORTED_PLATFORMS}")
    MESSAGE(STATUS "detected supported platforms: " ${SUPPORTED_PLATFORMS_STRING})
    LIST(GET SUPPORTED_PLATFORMS 0 def_platform)
    SET(PLATFORM def_platform CACHE STRING "Select the platform")
    SET_PROPERTY(CACHE PLATFORM PROPERTY STRINGS ${SUPPORTED_PLATFORMS})
ENDIF()
UNSET(PLATFORM_DIRS)

#At this point 'PLATFORM' should point to a valid plaform
#Before we include it though, we first need to:
#1) Hide the parameters of the previous platform.
#   Parameters of the currently selected platform are re-enabled
#   by the parameter-definitions of the platform itself
HIDE_PARAMETERS(PLATFORM_PARAM_LIST)
#2) Include helper macro's for platforms & chips
include(${PROJECT_SOURCE_DIR}/cmake/platform_macros.cmake)

#And finally: include the platform itself (if one is selected)
IF(NOT( PLATFORM STREQUAL "none"))
    MESSAGE(STATUS "selected platform: " ${PLATFORM})
    #Finally load the actual platform
    STRING(TOUPPER ${PLATFORM} __upper_name)
    SET(PLATFORM_PREFIX "PLATFORM_${__upper_name}")
    SET(${PLATFORM_PREFIX} ON)
    ADD_SUBDIRECTORY(${PLATFORM_PATH_${PLATFORM}} ${CMAKE_CURRENT_BINARY_DIR}/platform)
    ADD_CUSTOM_TARGET(
        jlink-open
        COMMAND ${JLinkExe} -speed 10000 -if SWD -Device ${JLINK_DEVICE} -AutoConnect 1
    )
ENDIF()

SET(CHIP_EXTRA_CHIPS_DIR "" CACHE STRING "Path to directory containing extra, out of tree chips implementations (optional)")
