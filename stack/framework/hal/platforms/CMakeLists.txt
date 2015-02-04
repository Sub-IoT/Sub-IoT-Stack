#Scan all platforms in the current directory for those that are
#compatible with the current toolchain
LIST_SUBDIRS(CUR_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
SET(SUPPORTED_PLATFORMS "")
FOREACH(dir ${CUR_DIRS})
    GET_REQUIRED_TOOLCHAIN(${dir} req_tc)
    IF(req_tc STREQUAL "${TOOLCHAIN}")
	LIST(APPEND SUPPORTED_PLATFORMS "${dir}")
    ENDIF()    
ENDFOREACH()
#If no platforms are compatible with the current toolchain, generate an error
IF(NOT SUPPORTED_PLATFORMS)
    MESSAGE(SEND_ERROR "There are no supported platforms for the selected toolchain ${TOOLCHAIN}")
ELSE()
#Otherwise add a parameter to allow the user to select the platform to use
    LIST(GET SUPPORTED_PLATFORMS 0 def_platform)
    SET (PLATFORM ${def_platform} CACHE STRING "Choose the platform to compile for.")
    SET_PROPERTY(CACHE PLATFORM PROPERTY STRINGS ${SUPPORTED_PLATFORMS})
ENDIF()
UNSET(CUR_DIRS)

#At this point 'PLATFORM' should point to a valid plaform
#Before we include it though, we first need to:
#1) Hide the parameters of the previous platform. 
#   Parameters of the currently selected platform are re-enabled
#   by the parameter-definitions of the platform itself
HIDE_PARAMETERS(PLATFORM_PARAM_LIST)
#2) Include helper macro's for platforms & chips
include(${CMAKE_SOURCE_DIR}/cmake/platform_macros.cmake)

#And finally: include the platform itself (if one is selected)
IF(NOT( PLATFORM STREQUAL "none"))
    #Finally load the actual platform
    STRING(TOUPPER ${PLATFORM} __upper_name)
    SET(PLATFORM_PREFIX "PLATFORM_${__upper_name}")
    SET(${PLATFORM_PREFIX} ON)
    ADD_SUBDIRECTORY(${PLATFORM})
ENDIF()
