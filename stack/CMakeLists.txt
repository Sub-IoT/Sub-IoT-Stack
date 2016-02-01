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

if(POLICY CMP0054)
    #Set CMP0054 to 'OLD'.
    #This was introduced with CMake 3.1, we're still at 2.8, so no need to update this and
    #potentially break the entire build chain until cmake 3.1 has become mainstream
    cmake_policy(SET CMP0054 OLD)
endif()

include(cmake/utils.cmake)
#First clear global variables that were stored in the cache to make them global
CLEAR_GLOBALS()

#Override link command to allow linker flags to be inserted wherever needed
SET_GLOBAL(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
SET_GLOBAL(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

CMAKE_MINIMUM_REQUIRED (VERSION 2.8.12)
#Allow the project name to be set from the cache
SET(PROJECT_NAME "OSS-7" CACHE STRING "The name of the project")
SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release")
SET_PROPERTY( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")

PROJECT (${PROJECT_NAME})

#Set the c standard to use
INSERT_C_FLAGS(BEFORE -std=c99 -fno-common)
#Retrieve the selected toolchain
GET_CURRENT_TOOLCHAIN(TOOLCHAIN)

#Then load the Framework
ADD_SUBDIRECTORY("framework")

#Load the file with platform-specific build commands
IF(EXISTS "${PROJECT_SOURCE_DIR}/cmake/platform_${PLATFORM}_macros.cmake")
    include ("cmake/platform_${PLATFORM}_macros.cmake")
ELSE()
    include ("cmake/platform_default_macros.cmake")
ENDIF()

#Then load the modules
ADD_SUBDIRECTORY("modules")
#And finally the applications
ADD_SUBDIRECTORY("apps")
#And tests
ADD_SUBDIRECTORY("tests")

