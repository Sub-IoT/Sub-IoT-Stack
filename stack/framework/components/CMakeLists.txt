#list all subdirectories to get a list of components
LIST_SUBDIRS(COMPONENTS ${CMAKE_CURRENT_SOURCE_DIR})
#add the individual libraries
FOREACH(COMPONENT ${COMPONENTS})
    LIST(FIND FRAMEWORK_OVERRIDE_LIBS "FRAMEWORK_${COMPONENT}" rmId)
    IF(rmId LESS 0) #component is not in rmlist
	#Yes, I know that this is discouraged by CMake because adding files does not cause cmake to be rerun...
	#Since the source tree is at this point far from stable however, I choose to ignore it
	FILE(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${COMPONENT}/*.c")
	ADD_LIBRARY(FRAMEWORK_${COMPONENT} OBJECT ${SOURCES})
        SET(FRAMEWORK_LIBS "${FRAMEWORK_LIBS};FRAMEWORK_${COMPONENT}" CACHE INTERNAL "")
    ENDIF()
ENDFOREACH()
