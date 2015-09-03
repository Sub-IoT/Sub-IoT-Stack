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

include(CMakeParseArguments)
# List all directories in the provided directory.
# By default, only the directories that are immediate 
# subdirs of <directory> are returned. If RECURSE is 
# specified, all subdirectories are recursively traversed.
# Usage:
#    LIST_SUBDIRS( <result_variable> <directory> [RECURSE]?)
# Based on:
#    http://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory
#
MACRO(LIST_SUBDIRS result dir)
  cmake_parse_arguments(__LS "RECURSE" "" "" ${ARGN})

  IF(__LS_RECURSE)
    FILE(GLOB_RECURSE __children ${dir}/[^.]*)
  ELSE()
    FILE(GLOB __children ${dir}/[^.]*)
  ENDIF()
  STRING(REGEX REPLACE "\\.git/[^;]+;?" "" __children "${__children}")
  UNSET(__LS_RECURSE)
  SET(${result} "")
  FOREACH(__child ${__children})
    IF(IS_DIRECTORY ${__child})
        LIST(APPEND ${result} ${__child})
    ELSE()
    	GET_FILENAME_COMPONENT(__dir "${__child}" PATH)
    	STRING(STRIP "${__dir}" __dir)
    	STRING(LENGTH "${__dir}" __len)
        IF((__len GREATER 0) AND (IS_DIRECTORY ${__dir}) AND (NOT(${__dir} STREQUAL ${dir})))
        LIST(FIND ${result} ${__dir} __id)
    	    IF(__id LESS 0)
    	       	LIST(APPEND ${result} ${__dir})
    	    ENDIF()
    	ENDIF()
    	UNSET(__len)
    	UNSET(__dir)
    	UNSET(__id)
    ENDIF()
  ENDFOREACH()
  UNSET(__children)
ENDMACRO()

# List all files in the provided directory.
# By default, only the files that are in <directory>
# are returned. If RECURSE is specified, all subdirectories 
# are recursively traversed.
# Usage:
#    LIST_FILES( <result_variable> <directory> [RECURSE]?)
# Based on:
#    http://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory
#
MACRO(LIST_FILES result dir)
  cmake_parse_arguments(__LS "RECURSE" "" "" ${ARGN})
  
  IF(__LS_RECURSE)
    FILE(GLOB_RECURSE __children RELATIVE ${dir} ${dir}/*)
  ELSE()
    FILE(GLOB __children RELATIVE ${dir} ${dir}/*)
  ENDIF()
  UNSET(__LS_RECURSE)
  
  SET(${result} "")
  FOREACH(__child ${__children})
    IF(NOT(IS_DIRECTORY ${dir}/${__child}))
        LIST(APPEND ${result} ${__child})
    ENDIF()
  ENDFOREACH()
  UNSET(__children)
ENDMACRO()

# Set 'CACHE' variable if specific conditions are met
# Usage:
#    SETCACHE_IF( <variable> <default_value> <type> <documentation_string> <condition>)
# 
MACRO(SETCACHE_IF var value type doc depends)
    SET(${var}_AVAILABLE 1)
    FOREACH(d ${depends})
      STRING(REGEX REPLACE " +" ";" __OPTION_DEP "${d}")
      IF(${__OPTION_DEP})
      ELSE()
        SET(${var}_AVAILABLE 0)
      ENDIF()
    ENDFOREACH()
    IF(${var}_AVAILABLE)
	IF(${var} MATCHES "^${var}$") #var NOT defined yet
	    SET(${var} ${value} CACHE ${type} "${doc}")
	ELSE() #var already defined (from an earlier set -> force it otherwise we can't convert the internal type to the actual type
	    SET(${var} "${${var}}" CACHE ${type} "${doc}" FORCE)
	ENDIF()
    ELSE()
	IF(${var} MATCHES "^${var}$") #var NOT yet defined yet, dump the default into the cache
	    SET(${var} ${value} CACHE INTERNAL "${doc}")
	ELSE() #var already defined but needs to be hidden -> use the internal type
	    SET(${var} "${${var}}" CACHE INTERNAL "${doc}")
	ENDIF()	
    ENDIF()
ENDMACRO()

# Set Option if specific conditions are met
# Usage:
#    SETOPTION_IF( <option> <documentation_string> <default> <condition>)
# 
MACRO(SETOPTION_IF option doc default depends)
    SETCACHE_IF(${option} ${default} BOOL ${doc} ${depends})
ENDMACRO()

# Hide a list of parameters from the 'cmake gui' options list. This command is intended to be used ONLY for
# Parameters that have been defined using 'SETCACHE_IF' or 'SETOPTION_IF'
# Usage:
#    HIDE_PARAMETERS(<variable> <list_of_parameters>)
# 
MACRO (HIDE_PARAMETERS param_list)
    FOREACH(param ${${param_list}})
	GET_PROPERTY(__param_doc CACHE ${param} PROPERTY HELPSTRING)
        SET(${param} "${${param}}" CACHE INTERNAL "${__param_doc}")
	UNSET(__param_doc)
    ENDFOREACH()
    SET(${param_list} "" CACHE INTERNAL "")
ENDMACRO()

# Store the current toolchain in <var>. The toolchain can ONLY be set by supplying -DCMAKE_TOOLCHAIN_FILE to 
# The cmake or cmake gui tool. 
# If ${CMAKE_TOOLCHAIN_FILE} is not defined (the default toolchain is used), <var> is set to the empty string
# If ${CMAKE_TOOLCHAIN_FILE} is defined (an alternative toolchain is specified), <var> is set to the `basename` 
# of the toolchain file, without the file extension. 
# If, for instance the toolchain file is set to 'cmake/toolchains/gcc-arm-embedded.cmake' <var> will be set 
# to 'gcc-arm-embedded'
# 
# Usage: GET_CURRENT_TOOLCHAIN(<var>)
#    
#
MACRO (GET_CURRENT_TOOLCHAIN var)
    IF(NOT (CMAKE_TOOLCHAIN_FILE STREQUAL ""))
	GET_FILENAME_COMPONENT(${var} "${CMAKE_TOOLCHAIN_FILE}" NAME_WE)
    ELSE()
	SET(${var} "")
    ENDIF()
ENDMACRO()

# Verify that the toolchain <tc> is used. If the specified toolchain is not used, an error is generated 
# explaining the issue to the user.
# Usage:
#    REQUIRE_TOOLCHAIN(<tc>)
#
MACRO (REQUIRE_TOOLCHAIN tc)
    GET_CURRENT_TOOLCHAIN(__toolchain)
    IF(NOT ( "${tc}" STREQUAL "${__toolchain}"))
	MESSAGE(SEND_ERROR "Required toolchain not selected. Required '${tc}' but was '${__toolchain}'")
    ENDIF()
    UNSET(__toolchain)
ENDMACRO()

# Generate a 'variable prefix' based on a specified <prefix> and a <name> and store it in <var>
# (This macro is used to generate, for instance the platform, chip, module and application prefixes)
# Usage:
#    GEN_PREFIX(<var> <prefix> <name>
#
MACRO(GEN_PREFIX var prefix name)
    STRING(TOUPPER "${name}" __upper)
    SET(${var} "${prefix}_${__upper}")
    UNSET(__upper)
ENDMACRO()

# Convert the specified paths to absolute paths and store the resulting paths in <var>. 
# Paths that are already absolute remain unaltered.
# Usage:
# RELATIVE_TO_ABSOLUTE(<var> <path1> [<path2> [<path3> ... ]])
#
MACRO(RELATIVE_TO_ABSOLUTE var)
    SET(__orig_paths ${ARGN})
    FOREACH(__path ${__orig_paths})
	GET_FILENAME_COMPONENT(__new_path "${__path}" ABSOLUTE)
	LIST(APPEND __new_paths "${__new_path}")
    ENDFOREACH()
    SET(${var} "${__new_paths}")
    UNSET(__new_paths)
    UNSET(__orig_paths)
ENDMACRO()

# Add one or more <value>'s to the ${CMAKE_SOURCE_DIR} directory property <property>. It should be noted that, by cmake 
# directory traversion rules, the new values are automatically propagated to any directories that are added 
# (using ADD_SUBDIRECTORY) AFTER the property has been set.  Directories that were included BEFORE the 
# property was set are NOT altered. This means that the added <value>'s are 'global' to the source 
# directories that must still be included. Already processed directories are not updated.
#
# Usage:
#    SET_GLOBAL_DIRECTORY_PROPERTY( <property> <value> <value> ...)
#
MACRO(SET_GLOBAL_DIRECTORY_PROPERTY property)
    GET_PROPERTY(__cur_props DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY ${property})
    SET_PROPERTY(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY ${property} ${__cur_props} ${ARGN})
    UNSET(__cur_props)
ENDMACRO()


# Register directories which can be included globally, used for instance for the public API of the framework 
#
# Usage:
#    EXPORT_GLOBAL_INCLUDE_DIRECTORIES(<dir> <dir> ...)
#
MACRO(EXPORT_GLOBAL_INCLUDE_DIRECTORIES)
    RELATIVE_TO_ABSOLUTE(__new_dirs ${ARGN})
    SET_PROPERTY(GLOBAL APPEND PROPERTY GLOBAL_INCLUDE_DIRECTORIES ${__new_dirs})
ENDMACRO()

# Export compile definitions which are used globally, used for example in an out-of-tree platform or chip
# to ensure the rest of the framework will be build with the correct definition
#
# Usage:
#    EXPORT_GLOBAL_COMPILE_DEFINITIONS(<dir> <dir> ...)
#
MACRO(EXPORT_GLOBAL_COMPILE_DEFINITIONS)
    SET_PROPERTY(GLOBAL APPEND PROPERTY GLOBAL_COMPILE_DEFINITIONS ${ARGN})
ENDMACRO()

# Add one or more compile <definitions>'s for the current directory and all directories above it. 
# This MACRO tries to mimic the behavior of 'ADD_DEFINITIONS' as closely as possible. Arguments starting with 
# '-D' and specifying a valid C preprocessor definition, are added to the 'COMPILE_DEFINITIONS' property 
# (as proper compile definitions). All other arguments are added to the 'COMPILE_OPTIONS' directory property.
# The specified definitions & flags become 'global' (according to the traversing rules specified by 
# 'SET_GLOBAL_DIRECTORY_PROPERTY').
#
# Usage:
#    GLOBAL_ADD_DEFINITIONS( <definition> <definition> ...)
#
MACRO(ADD_GLOBAL_DEFINITIONS)
    FOREACH(arg ${ARGN})
	IF( ${arg} MATCHES "^-D[a-zA-Z0-9_]+(=.+)?$")
	    #it's a proper definition string
	    STRING(REGEX REPLACE "^-D([a-zA-Z0-9_]+(=.+)?)$" "\\1" __repl ${arg})
	    LIST(APPEND __defs "${__repl}")
	    UNSET(__repl)
	ELSE()
	    LIST(APPEND __flags "${arg}")
	ENDIF()
    ENDFOREACH()
    SET_GLOBAL_DIRECTORY_PROPERTY(COMPILE_DEFINITIONS ${__defs})    
    SET_GLOBAL_DIRECTORY_PROPERTY(COMPILE_OPTIONS ${__flags})    
ENDMACRO()

# Add one or more compilation options for the current directory and all directories above it. The specified
# flags become 'global' flags (according to the traversing rules specified by 'SET_GLOBAL_DIRECTORY_PROPERTY').
#
# Usage:
#    GLOBAL_ADD_COMPILE_OPTIONS( <option> <option> ...)
#
MACRO(ADD_GLOBAL_COMPILE_OPTIONS)
    SET_GLOBAL_DIRECTORY_PROPERTY(COMPILE_OPTIONS ${ARGN})
ENDMACRO()

# Set a 'global' variable <var> to value <value>. 
# Global variables are stored in the cache to make them persistent across cmake files and 
# Automatically cleared when 'CLEAR_GLOBALS' is executed at the start of the next cmake run
#
# Usage:
#    SET_GLOBAL(<var> <value>
#
MACRO(SET_GLOBAL var value)
    SET(__vars ${__GLOBAL_VARS})
    LIST(APPEND __vars ${var})
    LIST(REMOVE_DUPLICATES __vars)
    SET(__GLOBAL_VARS "${__vars}" CACHE INTERNAL "")
    UNSET(__vars)
    SET(${var} "${value}" CACHE INTERNAL "")    
ENDMACRO()

# Clear all 'global' variables. This macro must be executed as a first command at the start of the the uppermost
# CMakeLists.txt file (before the project is defined)
#
# Usage:
#    CLEAR_GLOBALS()
#
MACRO(CLEAR_GLOBALS)
    FOREACH(var ${__GLOBAL_VARS})
	UNSET(${var} CACHE)
    ENDFOREACH()
    UNSET (__GLOBAL_VARS CACHE)
ENDMACRO()

# Add one or more linker flags at a specific point in the link command. This MACRO adds flags by operating on 
# the 'CMAKE_C_LINK_EXECUTABLE' and 'CMAKE_CXX_LINK_EXECUTABLE' variables. When using the 'Makefile' generator, 
# these variables are usually equal to:
# '<CMAKE_C(XX)_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>'.
#
# This MACRO can add linkerflags either righT BEFORE or AFTER a specific <tag_name>
#
# This MACRO requires CMAKE_C_LINK_EXECUTABLE and CMAKE_CXX_LINK_EXECUTABLE to be set and stores the modified 
# value as an internal cache varliable to make it persistent across file traversal.
#
# Usage:
#    INSERT_LINKER_FLAGS(BEFORE <tag_name> INSERT <flag> <flag> ...)
#	Inserts the specified <flag>'s BEFORE tag <tag_name> where <tag_name>
#	Is one of 'FLAGS', 'CMAKE_C_LINK_FLAGS', 'LINK_FLAGS', 'OBJECTS', 'TARGET' or 'LINK_LIBRARIES'
#    INSERT_LINKER_FLAGS(AFTER  <tag_name> INSERT <flag> <flag> ...)
#	Inserts the specified <flag>'s AFTER tag <tag_name> where <tag_name>
#	Is one of 'FLAGS', 'CMAKE_C_LINK_FLAGS', 'LINK_FLAGS', 'OBJECTS', 'TARGET' or 'LINK_LIBRARIES'
#
MACRO(INSERT_LINKER_FLAGS)
    cmake_parse_arguments(_ILF "" "BEFORE;AFTER" "INSERT" ${ARGN} )
    IF(NOT ${CMAKE_C_LINK_EXCUTABLE})	
        MESSAGE(FATAL_ERROR "INSERT_LINKER_FLAGS called before CMAKE_C_LINK_EXECUTABLE is defined")
    ENDIF()
    IF(NOT ${CMAKE_CXX_LINK_EXCUTABLE})	
        MESSAGE(FATAL_ERROR "INSERT_LINKER_FLAGS called before CMAKE_CXX_LINK_EXECUTABLE is defined")
    ENDIF()
    
    IF(_ILF_BEFORE AND _ILF_AFTER)
	MESSAGE(FATAL_ERROR "Exactly one of 'BEFORE' or 'AFTER' must be specified")
    ELSEIF(NOT (_ILF_BEFORE OR _ILF_AFTER))
	MESSAGE(FATAL_ERROR "Exactly one of 'BEFORE' or 'AFTER' must be specified")
    ELSE()
    	STRING(REGEX REPLACE ";" " " _ILF_INSERT "${_ILF_INSERT}")
    	IF(_ILF_BEFORE)
    	    IF(("${_ILF_BEFORE}" STREQUAL "CMAKE_C_COMPILER") OR ("${_ILF_BEFORE}" STREQUAL "CMAKE_CXX_COMPILER"))
    		MESSAGE(FATAL_ERROR "${_ILF_BEFORE} is not a valid tag_name for setting linker flags")
    	    ENDIF()
    	    SET(__findstr "<${_ILF_BEFORE}>")
    	ELSE()
    	    IF(("${_ILF_AFTER}" STREQUAL "CMAKE_C_COMPILER") OR ("${_ILF_AFTER}" STREQUAL "CMAKE_CXX_COMPILER"))
    		MESSAGE(FATAL_ERROR "${_ILF_AFTER} is not a valid tag_name for setting linker flags")
    	    ENDIF()
    	    SET(__findstr "<${_ILF_AFTER}>")
    	ENDIF()
    	
    	FOREACH(__var CMAKE_C_LINK_EXECUTABLE CMAKE_CXX_LINK_EXECUTABLE)
    	    STRING(FIND "${${__var}}" "${__findstr}" __pos)
    	    IF(${__pos} LESS 0)
    		MESSAGE(FATAL_ERROR "Cannot find tag ${__findstr} in ${__var}.")
    	    ENDIF()
	    IF(_ILF_AFTER)
		STRING(LENGTH "${__findstr}" __len)
		MATH(EXPR __pos "${__pos}+${__len}")
	    ENDIF()

	    STRING(SUBSTRING "${${__var}}" 0 ${__pos} __prefix)
	    STRING(SUBSTRING "${${__var}}" ${__pos} -1 __postfix)
    	    SET_GLOBAL(${__var} "${__prefix} ${_ILF_INSERT} ${__postfix}" CACHE INTERNAL "")

	    UNSET(__pos)
	    UNSET(__len)
	ENDFOREACH()
	UNSET(__findstr)
	UNSET(_ILF_INSERT)
	UNSET(_ILF_BEFORE)
	UNSET(_ILF_AFTER)
    ENDIF()
ENDMACRO()

# Add one or more flags to the C compiler command. This macro differs from GLOBAL_ADD_COMPILE_OPTIONS
# in the sense that GLOBAL_ADD_COMPILE_OPTIONS operates on the 'COMPILE_OPTIONS' property which means that 
# flags are added at the END of all compile flags (after the '-I' flags). This MACRO however operates on the
# 'CMAKE_C_FLAGS' variable which means that flags are added at the BEGIN of the compile command. 
# It should be noted that this MACRO turns CMAKE_C_FLAGS into a global variable
#
# Usage:
#    INSERT_C_FLAGS([BEFORE <flag> <flag> ...] [AFTER <flag> <flag> ...])
#	flags specified after 'BEFORE' are added right before the original CMAKE_C_FLAGS
#	flags specified after 'AFTER' are added right after the original CMAKE_C_FLAGS
#
MACRO(INSERT_C_FLAGS)
    cmake_parse_arguments(_ICF "" "" "BEFORE;AFTER" ${ARGN} )
    STRING(REGEX REPLACE ";" " " _ICF_BEFORE "${_ICF_BEFORE}")
    STRING(STRIP "${_ICF_BEFORE}" _ICF_BEFORE)
    STRING(REGEX REPLACE ";" " " _ICF_AFTER "${_ICF_AFTER}")
    STRING(STRIP "${_ICF_AFTER}" _ICF_AFTER)
    SET_GLOBAL(CMAKE_C_FLAGS "${_ICF_BEFORE} ${CMAKE_C_FLAGS} ${_ICF_AFTER}")
ENDMACRO()

# Add one or more flags to the C++ compiler command. This macro differs from GLOBAL_ADD_COMPILE_OPTIONS
# in the sense that GLOBAL_ADD_COMPILE_OPTIONS operates on the 'COMPILE_OPTIONS' property which means that 
# flags are added at the END of all compile flags (after the '-I' flags). This MACRO however operates on the
# 'CMAKE_CXX_FLAGS' variable which means that flags are added at the BEGIN of the compile command. 
# It should be noted that this MACRO turns CMAKE_CXX_FLAGS into a global variable
#
# Usage:
#    INSERT_CXX_FLAGS([BEFORE <flag> <flag> ...] [AFTER <flag> <flag> ...])
#	flags specified after 'BEFORE' are added right before the original CMAKE_C_FLAGS
#	flags specified after 'AFTER' are added right after the original CMAKE_C_FLAGS
#
MACRO(INSERT_CXX_FLAGS)
    cmake_parse_arguments(_ICF "" "" "BEFORE;AFTER" ${ARGN} )
    STRING(REGEX REPLACE ";" " " _ICF_BEFORE "${_ICF_BEFORE}")
    STRING(STRIP "${_ICF_BEFORE}" _ICF_BEFORE)
    STRING(REGEX REPLACE ";" " " _ICF_AFTER "${_ICF_AFTER}")
    STRING(STRIP "${_ICF_AFTER}" _ICF_AFTER)
    SET_GLOBAL(CMAKE_CXX_FLAGS "${_ICF_BEFORE} ${CMAKE_CXX_FLAGS} ${_ICF_AFTER}")
ENDMACRO()


# Get the toolchain required by the code in the specified <dir>.
# This MACRO is intended to be used mainly for platforms but may be useful in future
# to detect the toolchain required by other components as well.
#
# To detect the toolchain, MACRO looks for a 'toolchain' file in the <dir>. If it exists, this file
# is parsed to read the toolchain required by the code in the <dir>. The toolchain needs to be
# specified in the toolchain file according to the following format:
# 'toolchain=<required_toolchain>'
#
# If a valid toolchain file is found, the specified toolchain is stored in variable <var>
# if not, this variable is set to the empty string
#
#
# Usage:
#    GET_REQUIRED_TOOLCHAIN(<dir> <var>)
#
MACRO (GET_REQUIRED_TOOLCHAIN dir var)
    SET(${var} "")
    IF(EXISTS "${dir}/toolchain")
	FILE(STRINGS "${dir}/toolchain" __toolchains REGEX "^[ 	]*toolchain[ 	]*=[ 	]*[a-zA-Z0-9-]+.*$")
	LIST(LENGTH __toolchains __tclen)
	IF(${__tclen} GREATER 0)
	    LIST(GET __toolchains 0 __tc)
	    STRING(REGEX REPLACE "^[ 	]*toolchain[ 	]*=[ 	]*([a-zA-Z0-9-]+).*$" "\\1" __tc "${__tc}")
	    SET(${var} "${__tc}")
	ENDIF()
	UNSET(__tc)
	UNSET(__tclen)
	UNSET(__toolchains)
    ENDIF()
ENDMACRO()


# Generate a C header file based on the values of the supplied variable names.
# For each supplied variable a separate '#define' is generated. The manner in 
# which this is done depends on the 'type' of the variable. This macro considers 
# The following possible types:
#
#	STRING: the value of the variable is written as a C string. That is, for 
#	        each variable <var> the following statement is generated (with 
#		${<var>} the value of the variable):
#		    #define <var> "${<var>}"
#
#	ID:	the value of the variable is written as a C identifier. That is, for 
#	        each variable <var> the following statement is generated (with 
#		${<var>} the value of the variable):
#		    #define <var> ${<var>}
#	    	The only difference between ID and STRING is that for identifiers
#		no quotes (") are generated. This MACRO does not verify whether or
#		not ${var} is a valid C identifier
#
#	NUMBER:	the value of the variable is written as a number. Since both ID's
#		and numbers don't require quotes this is exactly the same as using ID.
#
#	BOOL:   a define statement is generated depending on whether or not <var>
#		evaluates to TRUE acording to the cmake rules. If <var> evaluates
#		to TRUE the following statement is generated:
#			#define <var>
#		otherwise, the #define statement is omitted.
#	
# Usage:
#    GEN_SETTINGS_HEADER([STRING <string_var> <string_var> ...] [ID <id_var> <id_var> ...] [BOOL <bool_var> <bool_var> ...] [NUMBER <number_var> <number_var> ...])
#
MACRO (GEN_SETTINGS_HEADER file)
    cmake_parse_arguments(_GSH "" "" "STRING;BOOL;ID;NUMBER" ${ARGN} )
    GET_FILENAME_COMPONENT(_header_guard "${file}" NAME)
    STRING(TOUPPER "${_header_guard}" _header_guard)
    STRING(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _header_guard "${_header_guard}")
    SET(_header_guard "__${_header_guard}__")
    
    FILE(WRITE  "${file}" "//Warning: This file is automatically generated by CMake. Do NOT EDIT this file directly !!\r\n")
    FILE(APPEND "${file}" "#ifndef ${_header_guard}\r\n")
    FILE(APPEND "${file}" "#define ${_header_guard}\r\n")
    
    FOREACH(var ${_GSH_STRING})
	FILE(APPEND "${file}" "#define ${var} \"${${var}}\"\r\n")	    
    ENDFOREACH()
    FOREACH(var ${_GSH_ID} ${_GSH_NUMBER})
	FILE(APPEND "${file}" "#define ${var} ${${var}}\r\n")	    
    ENDFOREACH()
    FOREACH(var ${_GSH_BOOL})
	IF(${var})
	    FILE(APPEND "${file}" "#define ${var}\r\n")	    
	ENDIF()
    ENDFOREACH()
    FILE(APPEND "${file}" "#endif //${_header_guard}\r\n")
    UNSET(_header_guard)
    UNSET(_GSH_BOOL)
    UNSET(_GSH_STRING)
    UNSET(_GSH_ID)
    UNSET(_GSH_NUMBER)
ENDMACRO()

# Filter a list of CACHE variables based on their type
# 
# Usage:
#    FILTER_CACHE_VARIABLES(<result> TYPES <type> <type> ... <VARS> <var> <var> ...)
#	<result> is the name of the variable in which to store the list of filtered variables
#	<type>   is one of the allowed CACHE types
#	<var>	 is a variable to check
#
MACRO (FILTER_CACHE_VARIABLES result )
    cmake_parse_arguments(_FCV "" "" "TYPES;VARS" ${ARGN} )
    SET(${result} "")
    FOREACH(var ${_FCV_VARS})    
	GET_PROPERTY(__type CACHE ${var} PROPERTY TYPE)
	LIST(FIND _FCV_TYPES "${__type}" __id)
	IF(NOT (${__id} LESS 0))
	    LIST(APPEND ${result} "${var}")
	ENDIF()
    ENDFOREACH()
    UNSET(__type)
    UNSET(__id)
    UNSET(__FCV_VARS)
    UNSET(__FCV_TYPES)
ENDMACRO()

# Helper MACRO used by (PLATFORM/FRAMEWORK)_HEADER_DEFINE
# You should normally not have to call this MACRO directly.
#
# This MACRO parses the arguments passed to it in more-or-less the same 
# fashion that GEN_SETTINGS_HEADER would and stores the parsed parameters
# in global variables <prefix>_<type> where <prefix> is the supplied prefix and
# <type> is the type of the variable (according to the parsing rules of 
# GEN_SETTINGS_HEADER.
#
# Usage:
#    PARSE_HEADER_VARS(<prefix> [STRING <string_var> <string_var> ...] [ID <id_var> <id_var> ...] [BOOL <bool_var> <bool_var> ...] [NUMBER <number_var> <number_var> ...])
#
MACRO(PARSE_HEADER_VARS prefix)
    cmake_parse_arguments(_PHV "" "" "STRING;BOOL;ID;NUMBER" ${ARGN} )
    IF(NOT ("${_PHV_STRING}" STREQUAL ""))
	SET_GLOBAL(${prefix}_STRING "${${prefix}_STRING};${_PHV_STRING}")
    ENDIF()
    IF(NOT ("${_PHV_BOOL}" STREQUAL ""))
	SET_GLOBAL(${prefix}_BOOL "${${prefix}_BOOL};${_PHV_BOOL}")
    ENDIF()
    IF(NOT ("${_PHV_NUMBER}" STREQUAL ""))
	SET_GLOBAL(${prefix}_NUMBER "${${prefix}_NUMBER};${_PHV_NUMBER}")
    ENDIF()
    IF(NOT ("${_PHV_ID}" STREQUAL ""))
	SET_GLOBAL(${prefix}_ID "${${prefix}_STRING};${_PHV_ID}")
    ENDIF()

    UNSET(_PHV_ID)
    UNSET(_PHV_BOOL)
    UNSET(_PHV_STRING)
    UNSET(_PHV_NUMBER)
ENDMACRO()

# Check if a macro with name <name> is available for the current source directory.
# If a macro with name <name> is available <var> is set to 'TRUE' otherwise
# it is set to 'FALSE'.
#
# Usage:
#    MACRO_AVAILABLE( <name> <var> )
#
MACRO(MACRO_AVAILABLE name var)
    SET(${var} "FALSE")
    SET(__dir ${CMAKE_CURRENT_SOURCE_DIR})
    GET_FILENAME_COMPONENT(__end_dir "${CMAKE_SOURCE_DIR}" PATH)
    WHILE(NOT (__dir STREQUAL "${__end_dir}"))
	GET_DIRECTORY_PROPERTY(__defined_macros DIRECTORY "${__dir}" MACROS)
	LIST(FIND __defined_macros "${name}" __macro_index)
	IF(NOT(__macro_index LESS 0))
	    SET(${var} "TRUE")
	    BREAK()
	ENDIF()
	GET_FILENAME_COMPONENT(__dir "${__dir}" PATH)
    ENDWHILE()
    UNSET(__end_dir)
    UNSET(__defined_macros)
    UNSET(__macro_index)
ENDMACRO()
