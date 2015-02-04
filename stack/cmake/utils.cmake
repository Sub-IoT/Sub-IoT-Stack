include(CMakeParseArguments)
# List all directories in the provided directory
# Usage:
#    LIST_SUBDIRS( <result_variable> <directory> )
# Based on:
#    http://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory
#
MACRO(LIST_SUBDIRS result dir)
  FILE(GLOB __children RELATIVE ${dir} ${dir}/*)
  SET(${result} "")
  FOREACH(__child ${__children})
    IF(IS_DIRECTORY ${dir}/${__child})
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

#Moved to framework/hal/platforms/CMakeLists.txt
#MACRO (GET_REQUIRED_TOOLCHAIN platform var)
#    SET(${var} "")
#    IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${platform}/toolchain")
#	FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${platform}/toolchain" __toolchains REGEX "^[ 	]*toolchain[ 	]*=[ 	]*[a-zA-Z0-9-]+.*$")
#	MESSAGE(AUTHOR_WARNING "toolchains: ${__toolchains}")
#	LIST(LENGTH __toolchains __tclen)
#	IF(${__tclen} GREATER 0)
#	    LIST(GET __toolchains 0 __tc)
#	    STRING(REGEX REPLACE "^[ 	]*toolchain[ 	]*=[ 	]*([a-zA-Z0-9-]+).*$" "\\1" __tc "${__tc}")
#	    SET(${var} "${__tc}")
#	ENDIF()
#	UNSET(__tc)
#	UNSET(__tclen)
#	UNSET(__toolchains)
#    ENDIF()
#ENDMACRO()

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
    IF(NOT (${CMAKE_TOOLCHAIN_FILE} STREQUAL ""))
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
    IF(NOT (${tc} STREQUAL ${__toolchain}))
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
#    MESSAGE(AUTHOR_WARNING "rel2abs called with params ${var} ${ARGN}")
    SET(__orig_paths ${ARGN})
    FOREACH(__path ${__orig_paths})
#	MESSAGE(AUTHOR_WARNING "processing path ${__path}")
	IF(IS_ABSOLUTE __path)
	    LIST(APPEND __new_paths "${__path}")
	ELSE()
	    LIST(APPEND __new_paths "${CMAKE_CURRENT_SOURCE_DIR}/${__path}")	
	ENDIF()
#	MESSAGE(AUTHOR_WARNING "current newpaths: ${__new_paths}")
    ENDFOREACH()
    SET(${var} "${__new_paths}")
    UNSET(__new_paths)
    UNSET(__orig_paths)
#    MESSAGE(AUTHOR_WARNING "rel2abs finished. result: ${${var}}")
ENDMACRO()

# Add one or more <value>'s to the specified directory property <property> both for the current directory and 
# all directories above it (stopping at the cmake source root directory). It should be noted that, by cmake 
# directory traversion rules, the new values are automatically propagated to any directories that are added 
# (using ADD_SUBDIRECTORY) AFTER the property has been set.  Directories that were included BEFORE the 
# property was set are NOT altered. This means that the added <value>'s are 'global' to the source 
# directories that must still be included. Already processed directories are not updated.
#
# Usage:
#    SET_GLOBAL_DIRECTORY_PROPERTY( <property> <value> <value> ...)
#
MACRO(SET_GLOBAL_DIRECTORY_PROPERTY property)
    SET(__cur_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    WHILE(NOT (__cur_dir STREQUAL "${CMAKE_SOURCE_DIR}"))
        GET_PROPERTY(__cur_props DIRECTORY "${__cur_dir}" PROPERTY ${property})
        SET_PROPERTY(DIRECTORY "${__cur_dir}" PROPERTY ${property} ${__cur_props} ${ARGN})
#	SET_PROPERTY(DIRECTORY "${__cur_dir}" PROPERTY ${property} ${ARGN})
        #GET_PROPERTY(__cur_props DIRECTORY "${__cur_dir}" PROPERTY ${property})
        #MESSAGE(AUTHOR_WARNING "Updated property ${property} on directory ${__cur_dir} to value :'${__cur_props}'")
	UNSET(__cur_props)
	STRING(FIND "${__cur_dir}" "/" __dir_sep_loc REVERSE)
	STRING(SUBSTRING "${__cur_dir}" 0 "${__dir_sep_loc}" __cur_dir)		
    ENDWHILE()
    UNSET(__cur_dir)
    GET_PROPERTY(__cur_props DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY ${property})
    SET_PROPERTY(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY ${property} ${__cur_props} ${ARGN})
#    SET_PROPERTY(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY ${property} ${ARGN})
    #GET_PROPERTY(__cur_props DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY ${property})
    #MESSAGE(AUTHOR_WARNING "Updated property ${property} on directory ${CMAKE_SOURCE_DIR} to value :'${__cur_props}'")
    UNSET(__cur_props)
ENDMACRO()


# Add one or more include <path>'s for the current directory and all directories above it. The specified 
# paths become 'global' paths (according to the traversing rules specified by 'SET_GLOBAL_DIRECTORY_PROPERTY').
# Paths are converted to absolute paths to ensure that they are interpreted correctly by the build system.
#
# It should also be noted that unlike the 'INCLUDE_DIRECTORIES' command, this MACRO does NOT automatically 
# add the binary path to the include directories. Only the paths specified are added.
#
# Usage:
#    GLOBAL_INCLUDE_DIRECTORIES( <path> <path> ...)
#
MACRO(GLOBAL_INCLUDE_DIRECTORIES)
    RELATIVE_TO_ABSOLUTE(__new_dirs ${ARGN})
    #MESSAGE(AUTHOR_WARNING "new_dirs: ${__new_dirs}")
    SET_GLOBAL_DIRECTORY_PROPERTY(INCLUDE_DIRECTORIES ${__new_dirs})
    #INCLUDE_DIRECTORIES(${__new_dirs})
    UNSET(__new_dirs)
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
#	MESSAGE(AUTHOR_WARNING "Clearing ${var}")
	UNSET(${var} CACHE)
    ENDFOREACH()
    UNSET (__GLOBAL_VARS CACHE)
ENDMACRO()

# Add one or more linker flags at a specific point in the link command. This MACRO adds flags by operating on 
# the 'CMAKE_C_LINK_EXECUTABLE'. When using the 'Makefile' generator, this variable is usually equal to 
# '<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>'.
# This MACRO adds linker flags either right BEFORE the <OBJECTS> or right AFTER the <LINK_LIBRARIES>.
#
# This MACRO requires CMAKE_C_LINK_EXECUTABLE to be set and stores the modified value as an internal cache
# varliable to make it persistent across file traversal.
#
# Usage:
#    INSERT_LINKER_FLAGS([BEFORE <flag> <flag> ...] [AFTER <flag> <flag> ...])
#	flags specified after 'BEFORE' are added right before the <OBJECTS> in the linker command
#	flags specified after 'AFTER' are added at the end of the linker command
#
MACRO(INSERT_LINKER_FLAGS)
    cmake_parse_arguments(_ILF "" "" "BEFORE;AFTER" ${ARGN} )
    IF(NOT ${CMAKE_C_LINK_EXCUTABLE})	
        MESSAGE(FATAL_ERROR "INSERT_LINKER_FLAGS called before CMAKE_C_LINK_EXECUTABLE is defined")
    ENDIF()
    IF(_ILF_BEFORE)
	STRING(REGEX REPLACE ";" " " _ILF_BEFORE "${_ILF_BEFORE}")
        STRING(FIND "${CMAKE_C_LINK_EXECUTABLE}" "<OBJECTS>" __pos)
        IF(${__pos} LESS 0)
    	    MESSAGE(FATAL_ERROR "CMAKE_C_LINK_EXECUTABLE is malformatted. Expected to find <OBJECTS>")
	ELSE()
	    STRING(SUBSTRING "${CMAKE_C_LINK_EXECUTABLE}" 0 ${__pos} __prefix)
	    STRING(SUBSTRING "${CMAKE_C_LINK_EXECUTABLE}" ${__pos} -1 __postfix)
	    SET(CMAKE_C_LINK_EXECUTABLE "${__prefix} ${_ILF_BEFORE} ${__postfix}" CACHE INTERNAL "")
	    UNSET(__prefix)
	    UNSET(__postfix)		
	ENDIF()
	UNSET(__pos)
    ENDIF()
    IF(_ILF_AFTER)
	STRING(REGEX REPLACE ";" " " _ILF_AFTER "${_ILF_AFTER}")
        SET(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE} ${_ILF_AFTER}" CACHE INTERNAL "")
    ENDIF()
    UNSET(_ILF_BEFORE)
    UNSET(_ILF_AFTER)
ENDMACRO()

# Add one or more flags to the compiler command. This macro differs from GLOBAL_ADD_COMPILE_OPTIONS
# in the sense that GLOBAL_ADD_COMPILE_OPTIONS operates on the 'COMPILE_OPTIONS' property which means that 
# flags are added at the END of all compile flags (after the '-I' flags). This MACRO however operates on the
# 'CMAKE_C_FLAGS' variable which means that flags are added at the BEGIN of the compile command. 
# It should be noted that this MACRO stores the CMAKE_C_FLAGS variable in the cache to make it persistent
# across files and that this variable should be cleared explicitly at the start of the main CMakeLists.txt 
# file
#
# Usage:
#    INSERT_COMPILE_FLAGS([BEFORE <flag> <flag> ...] [AFTER <flag> <flag> ...])
#	flags specified after 'BEFORE' are added right before the original CMAKE_C_FLAGS
#	flags specified after 'AFTER' are added right after the original CMAKE_C_FLAGS
#
MACRO(INSERT_COMPILE_FLAGS)
    cmake_parse_arguments(_ICF "" "" "BEFORE;AFTER" ${ARGN} )
    STRING(REGEX REPLACE ";" " " _ICF_BEFORE "${_ICF_BEFORE}")
    STRING(STRIP "${_ICF_BEFORE}" _ICF_BEFORE)
    STRING(REGEX REPLACE ";" " " _ICF_AFTER "${_ICF_AFTER}")
    STRING(STRIP "${_ICF_AFTER}" _ICF_AFTER)
    SET_GLOBAL(CMAKE_C_FLAGS "${_ICF_BEFORE} ${CMAKE_C_FLAGS} ${_ICF_AFTER}")
ENDMACRO()

# Get the toolchain required by the code in the <subdir> of the current directory.
# This MACRO is intended to be used mainly for platforms but may be useful in future
# to detect the toolchain required by other components as well.
#
# To detect the toolchain, MACRO looks for a 'toolchain' file in the <subdir>. If it exists, this file
# is parsed to read the toolchain required by the code in the <subdir>. The toolchain needs to be
# specified in the toolchain file according to the following format:
# 'toolchain=<required_toolchain>'
#
# If a valid toolchain file is found, the specified toolchain is stored in variable <var>
# if not, this variable is set to the empty string
#
#
# Usage:
#    GET_REQUIRED_TOOLCHAIN(<subdir> <var>)
#
MACRO (GET_REQUIRED_TOOLCHAIN subdir var)
    SET(${var} "")
    IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/toolchain")
	FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/toolchain" __toolchains REGEX "^[ 	]*toolchain[ 	]*=[ 	]*[a-zA-Z0-9-]+.*$")
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

