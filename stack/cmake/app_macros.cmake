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

#This file contains helper MACRO's that are only available to individual applications

# Declare a parameter for the application. Application parameters are 
# automaticall shown/hidden in the CMake GUI depending on whether or 
# not the application is enabled.
# 
# Application parameters are stored in the cache, just like regular parameters.
# This means that the properties of the parameter can be set in the normal manner
# (by using the SET_PROPERTY command)
#
# Usage:
#    APP_PARAM(<var> <default_value> <type> <doc_string>)
#	<var> 		is the name of the variable to set. By convention 
#			all application parameters should be prefixed by the 
#			${APP_PREFIX} variable (see examples below).
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
#	APP_PARAM(${APP_PREFIX}_STR "Default_Str" STRING "Parameter Explanation")
#		-Adds a STRING parameter '${APP_PREFIX}_STR' with default 
#		 value 'Default_Str' and explanation 'Parameter Explanation'
#		 The value of the parameter can be queried using:
#			${${APP_PREFIX}_STR}
#	
#	APP_PARAM(${APP_PREFIX}_LIST "item1" STRING "A List of items")
#	SET_PROPERTY(CACHE ${APP_PREFIX}_LIST PROPERTY STRINGS "item1;item2;item3")
#		-Adds a parameter '${APP_PREFIX}_LIST', with possible values
#		 'item1', 'item2' and 'item3'. Please note that this is done in
#		 exactly the same manner as for regular 'CACHE' parameters
MACRO(APP_PARAM var value type doc)
    SET(APP_PARAM_LIST "${APP_PARAM_LIST};${var}" CACHE INTERNAL "")
    #Set entry in cache if the application is being loaded
    SETCACHE_IF(${var} ${value} ${type} ${doc} ${APP_PREFIX})
ENDMACRO()

# Declare an option for the used application. Like application parameters,
# application options are automatically shown/hidden in the CMake GUI 
# depending on whether or not the application is enabled.
#
# As with application parameters, options are stored in the cache (just like
# regular CMake options)
#
# Usage:
#    APP_OPTION(<option> <doc_string> <default_value>)
#	<option>	is the name of the option to set. By convention 
#			all application options should be prefixed by the 
#			${APP_PREFIX} variable.
#
#	<doc_string>	the documentation string explaining the option
#
#       <default_value> the default value of the parameter.
#
#    In accordance with CMake's implementation of 'ADD_OPTION',
#    calling: 
#	APP_OPTION(<option> <doc_string> <default_value>)
#    is equivalent to calling:
#	APP_PARAM( <option> <default_value> BOOL <doc_string>
#
# Examples:
#	APP_OPTION(${APP_PREFIX}_OPT "OPT Explanation" FALSE)
#		-Adds a application option '{APP_PREFIX}_OPT' with 
#		 default value FALSE and explanation 'OPT Explanation'.
#		 The value of the option can be queried using:
#		    ${${APP_PREFIX}_OPT}
#
MACRO(APP_OPTION option doc default)
    APP_PARAM( ${option} ${default} BOOL ${doc})
ENDMACRO()

MACRO(__APP_BUILD_ON_LOCATION app_name flash_origin flash_length version_location)
	SET(ELF ${app_name}.elf)
	ADD_EXECUTABLE(${ELF} ${__APP_BUILD_SOURCES})
	
	TARGET_LINK_LIBRARIES(${ELF} PRIVATE ${__APP_BUILD_LIBS})
	
	GET_PROPERTY(__global_compile_definitions GLOBAL PROPERTY GLOBAL_COMPILE_DEFINITIONS)
	TARGET_COMPILE_DEFINITIONS(${ELF} PUBLIC ${__global_compile_definitions})
	
	GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
	TARGET_INCLUDE_DIRECTORIES(${ELF} PUBLIC ${__global_include_dirs})
	
	IF(APP_LINKER_SCRIPT)
		SET(__LINKER_SCRIPT ${APP_LINKER_SCRIPT})
	ELSE()
		SET(__LINKER_SCRIPT ${LINKER_SCRIPT})
	ENDIF()

	SET(FLASH_ORIGIN ${flash_origin})
	SET(FLASH_LENGTH ${flash_length})
	SET(VERSION_LOCATION ${version_location})
	IF(__LINKER_SCRIPT)
		CONFIGURE_FILE(${__LINKER_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${app_name}.ld)
		SET(__LINKER_SCRIPT_FLAG "-T${CMAKE_CURRENT_BINARY_DIR}/${app_name}.ld -Xlinker --print-memory-usage")
	ENDIF()

	SET_TARGET_PROPERTIES(${ELF} PROPERTIES LINK_FLAGS "${__LINKER_SCRIPT_FLAG} ${LINKER_FLAGS}")
	
	# extract hex files
	# when the system files are not stored a separate linker section but in RAM (default when not defined in platform) make sure these sections are not removed from the app hex
	IF((DEFINED PLATFORM_FS_SYSTEMFILES_IN_SEPARATE_LINKER_SECTION) AND PLATFORM_FS_SYSTEMFILES_IN_SEPARATE_LINKER_SECTION)
	  SET(REMOVE_SECTIONS "-R" ".d7ap_fs_metadata_section" "-R" ".d7ap_fs_permanent_files_section")
	  SET(ADD_SECTIONS "-j" ".d7ap_fs_metadata_section" "-j" ".d7ap_fs_permanent_files_section")
	ENDIF()
	
	ADD_CUSTOM_COMMAND(TARGET ${ELF} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O ihex ${ELF} ${app_name}-full.hex)
	ADD_CUSTOM_COMMAND(TARGET ${ELF} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
	  -O ihex
	  ${REMOVE_SECTIONS}
	  ${ELF} ${app_name}-app.hex
	)
	ADD_CUSTOM_COMMAND(TARGET ${ELF} POST_BUILD COMMAND ${CMAKE_OBJCOPY} 
	  -O binary 
	  ${REMOVE_SECTIONS}
	  ${ELF} ${app_name}-app.bin)
	ADD_CUSTOM_COMMAND(TARGET ${ELF} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
	  -O ihex
	  ${ADD_SECTIONS}
	  ${ELF} ${app_name}-eeprom-fs.hex
	)
	ADD_CUSTOM_COMMAND(TARGET ${ELF} POST_BUILD COMMAND ${CMAKE_OBJDUMP} --source -d ${ELF} > ${app_name}-disassembly.txt)

	# generate target for flashing application using jlink
	# TODO optional depending on platform?
	SET(BIN ${app_name}-app.bin)
	SET(HEX ${app_name}-full.hex)
	CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/jlink-flash.in ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-full.script)
	ADD_CUSTOM_TARGET(
		flash-${app_name}
		COMMAND ${JLinkExe} -speed 10000 -if SWD -CommandFile ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-full.script
		DEPENDS ${ELF}
	)
	
	SET(HEX ${app_name}-app.hex)
	CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/jlink-flash.in ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-app.script)
	ADD_CUSTOM_TARGET(
		flash-${app_name}-app
		COMMAND ${JLinkExe} -speed 10000 -if SWD -CommandFile ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-app.script
		DEPENDS ${ELF}
	)
	
	SET(HEX ${app_name}-eeprom-fs.hex)
	CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/jlink-flash.in ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-eeprom-fs.script)
	ADD_CUSTOM_TARGET(
		flash-${app_name}-eeprom-fs
		COMMAND ${JLinkExe} -speed 10000 -if SWD -CommandFile ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-eeprom-fs.script
		DEPENDS ${ELF}
	)
ENDMACRO()

MACRO(APP_BUILD)
    SET(options "")
    SET(oneValueArgs "NAME")
    SET(multiValueArgs SOURCES LIBS)
    CMAKE_PARSE_ARGUMENTS(__APP_BUILD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # define git revision so it can be used in the application
    INCLUDE(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
    GET_GIT_HEAD_REVISION(GIT_REFSPEC GIT_SHA1)
    CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/cmake/version.c.in" "${CMAKE_CURRENT_BINARY_DIR}/version.c")
    LIST(APPEND __APP_BUILD_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.c")

	
	
	IF(NOT DEFINED STANDALONE_FLASH_ORIGIN)
	  SET(STANDALONE_FLASH_ORIGIN 0)
	ENDIF()
	
	IF(NOT STANDALONE_FLASH_LENGTH)
	  SET(STANDALONE_FLASH_LENGTH 0)
	ENDIF()
	
	__APP_BUILD_ON_LOCATION(${__APP_BUILD_NAME} ${STANDALONE_FLASH_ORIGIN} ${STANDALONE_FLASH_LENGTH} 0)
	IF(PLATFORM_BUILD_BOOTLOADABLE_VERSION)
		IF(NOT BOOTLOADABLE_FLASH_ORIGIN)
		  MESSAGE(FATAL_ERROR "You should set BOOTLOADABLE_FLASH_ORIGIN when PLATFORM_BUILD_BOOTLOADABLE_VERSION=y")
		ENDIF()
		
		IF(NOT BOOTLOADABLE_FLASH_LENGTH)
		  MESSAGE(FATAL_ERROR "You should set BOOTLOADABLE_FLASH_LENGTH when PLATFORM_BUILD_BOOTLOADABLE_VERSION=y")
		ENDIF()

		IF(NOT BOOTLOADABLE_VERSION_LOCATION)
		  MESSAGE("You should set BOOTLOADABLE_VERSION_LOCATION when PLATFORM_BUILD_BOOTLOADABLE_VERSION=y to be able to get the version from ROM")
		ENDIF()

		__APP_BUILD_ON_LOCATION("${__APP_BUILD_NAME}-bootloadable" ${BOOTLOADABLE_FLASH_ORIGIN} ${BOOTLOADABLE_FLASH_LENGTH} ${BOOTLOADABLE_VERSION_LOCATION})
	ENDIF()

    #INSERT_LINKER_FLAGS(BEFORE OBJECTS INSERT "-T${LINKER_SCRIPT}")
    #INSERT_LINKER_FLAGS(AFTER OBJECTS INSERT "-Xlinker  -Map=.map")
ENDMACRO()


# Add one or more CMAKE variables as '#define' statements to the "app_defs.h"
# generated by the APP_BUILD_SETTINGS_FILE MACRO. This MACRO adopts the same
# variable types used by the GEN_SETTINGS_HEADER MACRO. See the explanation of that macro
# in utils.cmake for more explanation.
#
# Usage:
#    APP_HEADER_DEFINE([STRING <string_var> <string_var> ...] [ID <id_var> <id_var> ...] [BOOL <bool_var> <bool_var> ...] [NUMBER <number_var> <number_var> ...])
#
MACRO(APP_HEADER_DEFINE)
    PARSE_HEADER_VARS("APP_EXTRA_DEFS" ${ARGN})
ENDMACRO()

# Construct a "app_defs.h" header file in the binary 'app' directory containing 
# The various (cmake) settings for the app. By default an 'empty' header file is generated.
# Settings can be added to this file by calling the APP_HEADER_DEFINES macro.
#
# Usage:
#    APP_BUILD_SETTINGS_FILE()
#
MACRO(APP_BUILD_SETTINGS_FILE)
    GEN_SETTINGS_HEADER("${CMAKE_CURRENT_BINARY_DIR}/app_defs.h" 
			STRING "${APP_EXTRA_DEFS_STRING}" 
			BOOL "${APP_EXTRA_DEFS_BOOL}" 
			ID "${APP_EXTRA_DEFS_ID}"
			NUMBER "${APP_EXTRA_DEFS_NUMBER}" 
			)			
    SET_GLOBAL(APP_EXTRA_DEFS_STRING "")
    SET_GLOBAL(APP_EXTRA_DEFS_BOOL "")
    SET_GLOBAL(APP_EXTRA_DEFS_ID "")
    SET_GLOBAL(APP_EXTRA_DEFS_NUMBER "")
ENDMACRO()
