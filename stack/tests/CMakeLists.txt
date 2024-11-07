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

#Add the 'BUILD_UNIT_TESTS' and 'INCLUDE_GTEST' options
OPTION(BUILD_UNIT_TESTS "Build unit tests applications" ON)
OPTION(INCLUDE_GTEST "Download and include google test" OFF)

SET(TESTS_EXTRA_TESTS_DIR "" CACHE STRING "Path to directory containing extra, out of tree tests (optional)")

LIST_SUBDIRS(TEST_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
IF(NOT(TESTS_EXTRA_TESTS_DIR STREQUAL ""))
    LIST_SUBDIRS(EXTRA_TESTS_DIRS ${TESTS_EXTRA_TESTS_DIR})
    LIST(APPEND TEST_DIRS ${EXTRA_TESTS_DIRS})
ENDIF()

#Add conditional options for all subdirs (which are individual applications)
FOREACH(__dir ${TEST_DIRS})
    GET_FILENAME_COMPONENT(TEST_NAME ${__dir} NAME) # strip full path keeping only test name
    STRING(TOUPPER ${TEST_NAME} __upper_name)
    SETOPTION_IF(TEST_${__upper_name} "Build unit test application ${__dir}" OFF BUILD_UNIT_TESTS)
    UNSET(__upper_name)
ENDFOREACH()

# Make google test available if INCLUDE_GTEST is set
IF(INCLUDE_GTEST)
    # GoogleTest requires at least C++14
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1 # g++ 11
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    # set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
ENDIF()


#Finally Load the individual applications
FOREACH(__dir ${TEST_DIRS})
    GET_FILENAME_COMPONENT(TEST_NAME ${__dir} NAME) # strip full path keeping only test name
    GEN_PREFIX(TEST_PREFIX "TEST" ${TEST_NAME})
    IF(BUILD_UNIT_TESTS AND ${TEST_PREFIX})
        ADD_SUBDIRECTORY(${__dir} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
    ENDIF()
    UNSET(TEST_PREFIX)
ENDFOREACH()
