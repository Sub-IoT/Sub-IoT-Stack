include(cmake/utils.cmake)
#First clear global variables that were stored in the cache to make them global
CLEAR_GLOBALS()

#Override link command to allow linker flags to be inserted wherever needed
SET_GLOBAL(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
SET_GLOBAL(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
#Allow the project name to be set from the cache
SET(PROJECT_NAME "iFest" CACHE STRING "The name of the project")
SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release")
SET_PROPERTY( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")
#Need to enable CXX here because CMake complains when CXX is only enabled when the 'sim' platform is loaded
PROJECT (${PROJECT_NAME})

#Set the c standard to use
INSERT_C_FLAGS(BEFORE -std=c99 -fno-common)
#Retrieve the selected toolchain
GET_CURRENT_TOOLCHAIN(TOOLCHAIN)

#Then load the Framework
ADD_SUBDIRECTORY("framework")

#Load the file with platform-specific build commands
IF(EXISTS "${CMAKE_SOURCE_DIR}/cmake/platform_${PLATFORM}_macros.cmake")
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

