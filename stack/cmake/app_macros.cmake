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

    IF(PLATFORM_USE_BOOTLOADER)
        SET(__APP_BUILD_NAME ${__APP_BUILD_NAME}_bootloader)
    ENDIF()

    #INSERT_LINKER_FLAGS(BEFORE OBJECTS INSERT "-T${LINKER_SCRIPT}")
    #INSERT_LINKER_FLAGS(AFTER OBJECTS INSERT "-Xlinker  -Map=.map")

    SET(ELF ${__APP_BUILD_NAME}.elf)
    ADD_EXECUTABLE(${ELF} ${__APP_BUILD_SOURCES})

    TARGET_LINK_LIBRARIES(${ELF} ${__APP_BUILD_LIBS})

    GET_PROPERTY(__global_compile_definitions GLOBAL PROPERTY GLOBAL_COMPILE_DEFINITIONS)
    TARGET_COMPILE_DEFINITIONS(${ELF} PUBLIC ${__global_compile_definitions})

    IF(LINKER_SCRIPT)
        SET(__LINKER_SCRIPT_FLAG "-T${LINKER_SCRIPT}")
    ENDIF()

    SET_TARGET_PROPERTIES(${ELF} PROPERTIES LINK_FLAGS "${__LINKER_SCRIPT_FLAG} ${LINKER_FLAGS}")

#    TODO not used for now
#    IF(${PLATFORM_BUILD_BOOTLOADABLE_VERSION})
#        SET(BOOTLOADABLE_ELF ${__APP_BUILD_NAME}_bootloadable.elf)
#        SET(BOOTLOADABLE_BIN ${__APP_BUILD_NAME}_bootloadable.bin)
#        ADD_EXECUTABLE(${BOOTLOADABLE_ELF} ${__APP_BUILD_SOURCES})
#        TARGET_LINK_LIBRARIES(${BOOTLOADABLE_ELF} -Wl,--whole-archive ${__APP_BUILD_LIBS}  -Wl,--no-whole-archive)
#        TARGET_COMPILE_DEFINITIONS(${BOOTLOADABLE_ELF} PUBLIC ${__global_compile_definitions})
#        SET_TARGET_PROPERTIES(${BOOTLOADABLE_ELF} PROPERTIES LINK_FLAGS "-T${LINKER_SCRIPT_BOOTLOADABLE} ${LINKER_FLAGS}")
#        ADD_CUSTOM_COMMAND(TARGET ${BOOTLOADABLE_ELF} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O binary ${BOOTLOADABLE_ELF} ${BOOTLOADABLE_BIN})
#    ENDIF()

    # extract hex files
    ADD_CUSTOM_COMMAND(TARGET ${ELF} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O ihex ${ELF} ${__APP_BUILD_NAME}-full.hex)
    ADD_CUSTOM_COMMAND(TARGET ${ELF} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
      -O ihex
      -R .d7ap_fs_systemfiles_header_data
      -R .d7ap_fs_systemfiles_data
      ${ELF} ${__APP_BUILD_NAME}-app.hex
    )
    ADD_CUSTOM_COMMAND(TARGET ${ELF} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
      -O ihex
      -j .d7ap_fs_systemfiles_header_data
      -j .d7ap_fs_systemfiles_data
      ${ELF} ${__APP_BUILD_NAME}-eeprom-fs.hex
    )

    # generate target for flashing application using jlink
    # TODO optional depending on platform?
    SET(HEX ${__APP_BUILD_NAME}-full.hex)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/jlink-flash.in ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-full.script)
    ADD_CUSTOM_TARGET(
        flash-${__APP_BUILD_NAME}
        COMMAND ${JLinkExe} -speed 10000 -if SWD -CommandFile ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-full.script
        DEPENDS ${ELF}
    )

    SET(HEX ${__APP_BUILD_NAME}-app.hex)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/jlink-flash.in ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-app.script)
    ADD_CUSTOM_TARGET(
        flash-${__APP_BUILD_NAME}-app
        COMMAND ${JLinkExe} -speed 10000 -if SWD -CommandFile ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-app.script
        DEPENDS ${ELF}
    )

    SET(HEX ${__APP_BUILD_NAME}-eeprom-fs.hex)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/jlink-flash.in ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-eeprom-fs.script)
    ADD_CUSTOM_TARGET(
        flash-${__APP_BUILD_NAME}-eeprom-fs
        COMMAND ${JLinkExe} -speed 10000 -if SWD -CommandFile ${CMAKE_CURRENT_BINARY_DIR}/jlink-flash-eeprom-fs.script
        DEPENDS ${ELF}
    )
ENDMACRO()
