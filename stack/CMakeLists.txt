include(cmake/utils.cmake)
#First clear global variables that were stored in the cache to make them global
CLEAR_GLOBALS()

CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
#Allow the project name to be set from the cache
SET(PROJECT_NAME "iFest" CACHE STRING "The name of the project")
SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release")
SET_PROPERTY( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")
PROJECT (${PROJECT_NAME} LANGUAGES C)

#Set the c standard to use
INSERT_COMPILE_FLAGS(BEFORE -std=c99 -fno-common)
#Retrieve the selected toolchain
GET_CURRENT_TOOLCHAIN(TOOLCHAIN)

#Then load the Framework
ADD_SUBDIRECTORY("framework")
#Then load the modules
ADD_SUBDIRECTORY("modules")
#And finally the applications
ADD_SUBDIRECTORY("apps")
#And tests
#ADD_SUBDIRECTORY("tests")

