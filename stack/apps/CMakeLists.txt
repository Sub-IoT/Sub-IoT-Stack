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

SET(APP_EXTRA_APPS_DIR "" CACHE STRING "Path to directory containing extra, out of tree apps (optional)")

include(${PROJECT_SOURCE_DIR}/cmake/app_macros.cmake)

#Add the 'BUILD_APPLICATIONS' option
OPTION(BUILD_APPLICATIONS "Build Applications" ON)
LIST_SUBDIRS(APP_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
IF(NOT(APP_EXTRA_APPS_DIR STREQUAL ""))
    LIST_SUBDIRS(EXTRA_APP_DIRS ${APP_EXTRA_APPS_DIR})
    LIST(APPEND APP_DIRS ${EXTRA_APP_DIRS})
ENDIF()

#Add conditional options for all subdirs (which are individual applications)
FOREACH(__dir ${APP_DIRS})
    GET_FILENAME_COMPONENT(APP_NAME ${__dir} NAME) # strip full path keeping only app name
    STRING(TOUPPER ${APP_NAME} __upper_name)
    SETOPTION_IF(APP_${__upper_name} "Build Application ${__dir}" OFF BUILD_APPLICATIONS)
    UNSET(__upper_name)
ENDFOREACH()

#Hide the parameters declared by the applications during the previous run
#Applications that are enabled will re-enable their parameters during declaration (eg by calling APP_PARAM)
HIDE_PARAMETERS(APP_PARAM_LIST)


#Finally Load the individual applications
FOREACH(__dir ${APP_DIRS})
    GET_FILENAME_COMPONENT(APP_NAME ${__dir} NAME) # strip full path keeping only app name
    GEN_PREFIX(APP_PREFIX "APP" ${APP_NAME})
    IF(BUILD_APPLICATIONS AND ${APP_PREFIX})
        ADD_SUBDIRECTORY(${__dir} ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME})
        SET(__BUILDING_APP TRUE)
    ENDIF()
    UNSET(APP_PREFIX)
ENDFOREACH()

IF(NOT __BUILDING_APP)
  MESSAGE("No application is configured, you might want to include an application (by adding -DAPP_GATEWAY=y for example)")
ENDIF()
