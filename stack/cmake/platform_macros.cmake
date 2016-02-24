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

#This file contains helper MACRO's that are only available to platforms and chips

# Declare a parameter for the used platform. Platform parameters are 
# automaticall shown/hidden in the CMake GUI depending on whether or 
# not the platform is enabled.
# 
# Platform parameters are stored in the cache, just like regular parameters.
# This means that the properties of the parameter can be set in the normal manner
# (by using the SET_PROPERTY command)
#
# Usage:
#    PLATFORM_PARAM(<var> <default_value> <type> <doc_string>)
#	<var> 		is the name of the variable to set. By convention 
#			all platform parameters should be prefixed by the 
#			${PLATFORM_PREFIX} variable (see examples below).
#
#       <default_value> the default value of the parameter.
#
#	<type>		the type of the parameter. Any valid CMake cache <type>
#			except INTERNAL. At the time of writing these are the valid
#			types:
#				FILEPATH = File chooser dialog.
#				PATH     = Directory chooser dialog.
#				STRING   = Arbitrary string.
#				BOOL     = Boolean ON/OFF checkbox.
#				See http://www.cmake.org/cmake/help/v3.0/command/set.html
#
#	<doc_string>	the documentation string explaining the parameter
#
# Examples:
#	PLATFORM_PARAM(${PLATFORM_PREFIX}_STR "Default_Str" STRING "Parameter Explanation")
#		-Adds a STRING parameter '${PLATFORM_PREFIX}_STR' with default 
#		 value 'Default_Str' and explanation 'Parameter Explanation'
#		 The value of the parameter can be queried using:
#			${${PLATFORM_PREFIX}_STR}
#	
#	PLATFORM_PARAM(${PLATFORM_PREFIX}_LIST "item1" STRING "A List of items")
#	SET_PROPERTY(CACHE ${PLATFORM_PREFIX}_LIST PROPERTY STRINGS "item1;item2;item3")
#		-Adds a parameter '${PLATFORM_PREFIX}_LIST', with possible values
#		 'item1', 'item2' and 'item3'. Please note that this is done in
#		 exactly the same manner as for regular 'CACHE' parameters
#
MACRO(PLATFORM_PARAM var value type doc)
    SET(PLATFORM_PARAM_LIST "${PLATFORM_PARAM_LIST};${var}" CACHE INTERNAL "")
    #Set entry in cache if this the platform us being used is being built
    SETCACHE_IF(${var} ${value} ${type} ${doc} ${PLATFORM_PREFIX})
ENDMACRO()

# Declare an options for the used platform. Like platform parameters,
# platform options are automatically shown/hidden in the CMake GUI 
# depending on whether or not the platform is enabled.
#
# As with platform parameters, options are stored in the cache (just like
# regular CMake options)
#
# Usage:
#    PLATFORM_OPTION(<option> <doc_string> <default_value>)
#	<option>	is the name of the option to set. By convention 
#			all platform options should be prefixed by the 
#			${PLATFORM_PREFIX} variable.
#
#	<doc_string>	the documentation string explaining the option
#
#       <default_value> the default value of the parameter.
#
#    In accordance with CMake's implementation of 'ADD_OPTION',
#    calling: 
#	PLATFORM_OPTION(<option> <doc_string> <default_value>)
#    is equivalent to calling:
#	PLATFORM_PARAM( <option> <default_value> BOOL <doc_string>
#
# Examples:
#	PLATFORM_OPTION(${PLATFORM_PREFIX}_OPT "OPT Explanation" FALSE)
#		-Adds a platform option '{PLATFORM_PREFIX}_OPT' with 
#		 default value FALSE and explanation 'OPT Explanation'.
#		 The value of the option can be queried using:
#		    ${${PLATFORM_PREFIX}_OPT}
#
MACRO(PLATFORM_OPTION option doc default)
    PLATFORM_PARAM( ${option} ${default} BOOL ${doc})
ENDMACRO()

# Include drivers for a chip <chip> into the current platform. 
# If CHIP_EXTRA_CHIPS_DIR is set the driver implementation will be searched in 
# the specified path first. If not found the default path 'framework/hal/chips' 
# will be searched. Each chip is in a separate subdirectory which must contain a 'CMakeLists.txt' file that defines 
# an object library with name '${CHIP_LIBRARY_NAME}'.
##
# Usage:
#    ADD_CHIP(<chip>)
#	<chip>		The chip directory to include
#
MACRO(ADD_CHIP chip)
    GEN_PREFIX(CHIP_LIBRARY_NAME "CHIP" ${chip})
    STRING(TOUPPER "${chip}" __uchip)
    SET(USE_${__uchip} TRUE)
    PLATFORM_HEADER_DEFINE(BOOL USE_${__uchip})
    UNSET(__uchip)
    UNSET(CHIP_DIR)
    IF(NOT(CHIP_EXTRA_CHIPS_DIR STREQUAL ""))
        IF(EXISTS ${CHIP_EXTRA_CHIPS_DIR}/${chip} AND IS_DIRECTORY ${CHIP_EXTRA_CHIPS_DIR}/${chip})
            SET(CHIP_DIR ${CHIP_EXTRA_CHIPS_DIR}/${chip})
        ENDIF()
    ENDIF()
    IF(NOT CHIP_DIR)
        SET(CHIP_DIR ${PROJECT_SOURCE_DIR}/framework/hal/chips/${chip})
    ENDIF()
    ADD_SUBDIRECTORY("${CHIP_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/chips/${chip}")
    ADD_DEPENDENCIES(PLATFORM ${CHIP_LIBRARY_NAME})
    GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
    TARGET_INCLUDE_DIRECTORIES(${CHIP_LIBRARY_NAME} PUBLIC ${__global_include_dirs})
    GET_PROPERTY(__global_compile_definitions GLOBAL PROPERTY GLOBAL_COMPILE_DEFINITIONS) 
    TARGET_COMPILE_DEFINITIONS(${CHIP_LIBRARY_NAME} PUBLIC ${__global_compile_definitions}) 
    SET_GLOBAL(PLATFORM_CHIP_LIBS "${PLATFORM_CHIP_LIBS};${CHIP_LIBRARY_NAME}")   
    UNSET(CHIP_LIBRARY_NAME)
    MESSAGE(STATUS "Added chip " ${chip})
ENDMACRO()

# Export directories to be included by the platform target. This MACRO is intended to be used 
# in the CMakeLists.txt file of individual chips and allows each indiviual chip to 
# export interfaces to the platform code (but NOT the entire source tree). 
#
# This is, for instance, usefull if platform specific code needs access to 'private' 
# chip-specific headers.
#
# Usage:
#    EXPORT_PLATFORM_INCLUDE_DIRECTORIES(<dir> <dir> ...)
#
MACRO(EXPORT_PLATFORM_INCLUDE_DIRECTORIES)
    RELATIVE_TO_ABSOLUTE(__paths ${ARGN})
    SET_PROPERTY(GLOBAL APPEND PROPERTY PLATFORM_INCLUDE_DIRECTORIES ${__paths})
ENDMACRO()

# Add one or more CMAKE variables as '#define' statements to the "platform_defs.h"
# generated by the PLATFORM_BUILD_SETTINGS_FILE MACRO. This MACRO adopts the same
# variable types used by the GEN_SETTINGS_HEADER MACRO. See the explanation of that macro
# in utils.cmake for more explanation.
#
# Usage:
#    PLATFORM_HEADER_DEFINE([STRING <string_var> <string_var> ...] [ID <id_var> <id_var> ...] [BOOL <bool_var> <bool_var> ...] [NUMBER <number_var> <number_var> ...])
#
MACRO(PLATFORM_HEADER_DEFINE)
    PARSE_HEADER_VARS("PLATFORM_EXTRA_DEFS" ${ARGN})
ENDMACRO()

# Construct a "platform_defs.h" header file in the binary 'platform' directory containing 
# The various settings for the platform. By default the following settings are included:
#
# -The included CHIPS (a #define USE_<chip> is added for each chip included with ADD_CHIP)
# -PLATFORM  (a #define PLATFORM "<platform_name>" )
# -PLATFORM_PREFIX (a define PLATFORM_<platform_name>)
#
# Additional settings can be added by calling the PLATFORM_HEADER_DEFINES macro
#
# Usage:
#    PLATFORM_BUILD_SETTINGS_FILE()
#
MACRO(PLATFORM_BUILD_SETTINGS_FILE)
    PLATFORM_HEADER_DEFINE(STRING PLATFORM BOOL ${PLATFORM_PREFIX})
    GEN_SETTINGS_HEADER("${CMAKE_CURRENT_BINARY_DIR}/platform_defs.h" 
			STRING "${PLATFORM_EXTRA_DEFS_STRING}" 
			BOOL "${PLATFORM_EXTRA_DEFS_BOOL}" 
			ID "${PLATFORM_EXTRA_DEFS_ID}"
			NUMBER "${PLATFORM_EXTRA_DEFS_NUMBER}" 
			)
			
    SET_GLOBAL(PLATFORM_EXTRA_DEFS_STRING "")
    SET_GLOBAL(PLATFORM_EXTRA_DEFS_BOOL "")
    SET_GLOBAL(PLATFORM_EXTRA_DEFS_ID "")
    SET_GLOBAL(PLATFORM_EXTRA_DEFS_NUMBER "")
ENDMACRO()
