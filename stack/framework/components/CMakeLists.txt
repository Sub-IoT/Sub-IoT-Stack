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

#list all subdirectories to get a list of components
LIST_SUBDIRS(COMPONENTS ${CMAKE_CURRENT_SOURCE_DIR})
#add the individual libraries
FOREACH(COMPONENT ${COMPONENTS})
    GET_FILENAME_COMPONENT(COMPONENT_NAME ${COMPONENT} NAME) # strip full path keeping only component name
    SET(COMPONENT_LIBRARY_NAME "FRAMEWORK_COMPONENT_${COMPONENT_NAME}")
    LIST(FIND FRAMEWORK_OVERRIDE_LIBS ${COMPONENT_LIBRARY_NAME} rmId)
    IF(rmId LESS 0)
        LIST(FIND FRAMEWORK_EXCLUDE_LIBS ${COMPONENT_LIBRARY_NAME} rmId)
    ENDIF()
    IF(rmId LESS 0) #component is not in rmlist
        ADD_SUBDIRECTORY(${COMPONENT})
        ADD_DEPENDENCIES(${COMPONENT_LIBRARY_NAME} PLATFORM)
        SET_GLOBAL(FRAMEWORK_LIBS "${FRAMEWORK_LIBS};${COMPONENT_LIBRARY_NAME}")
        GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
        TARGET_INCLUDE_DIRECTORIES(${COMPONENT_LIBRARY_NAME} PUBLIC	${__global_include_dirs})
        GET_PROPERTY(__global_compile_definitions GLOBAL PROPERTY GLOBAL_COMPILE_DEFINITIONS)
        TARGET_COMPILE_DEFINITIONS(${COMPONENT_LIBRARY_NAME} PUBLIC ${__global_compile_definitions})
    ENDIF()
    UNSET(COMPONENT_LIBRARY_NAME)
ENDFOREACH()
