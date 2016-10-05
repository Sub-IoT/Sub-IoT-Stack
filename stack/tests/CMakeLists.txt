# 
# OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
# lowpower wireless sensor communication
#
# Copyright 2016 University of Antwerp
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

#Add the 'BUILD_UNIT_TESTS' option
OPTION(BUILD_UNIT_TESTS "Build unit tests applications" ON)
LIST_SUBDIRS(TEST_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

#Add conditional options for all subdirs (which are individual applications)
FOREACH(__dir ${TEST_DIRS})
    GET_FILENAME_COMPONENT(TEST_NAME ${__dir} NAME) # strip full path keeping only test name
    STRING(TOUPPER ${TEST_NAME} __upper_name)
    SETOPTION_IF(TEST_${__upper_name} "Build unit test application ${__dir}" OFF BUILD_UNIT_TESTS)
    UNSET(__upper_name)
ENDFOREACH()

#Finally Load the individual applications
FOREACH(__dir ${TEST_DIRS})
    GET_FILENAME_COMPONENT(TEST_NAME ${__dir} NAME) # strip full path keeping only test name
    GEN_PREFIX(TEST_PREFIX "TEST" ${TEST_NAME})
    IF(BUILD_UNIT_TESTS AND ${TEST_PREFIX})
        ADD_SUBDIRECTORY(${__dir} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
    ENDIF()
    UNSET(TEST_PREFIX)
ENDFOREACH()
