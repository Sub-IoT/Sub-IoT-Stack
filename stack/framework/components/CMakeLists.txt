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

#list all subdirectories to get a list of components
LIST_SUBDIRS(COMPONENTS ${CMAKE_CURRENT_SOURCE_DIR})
#add the individual libraries
FOREACH(COMPONENT ${COMPONENTS})
    SET(COMPONENT_LIBRARY_NAME "FRAMEWORK_${COMPONENT}")
    LIST(FIND FRAMEWORK_OVERRIDE_LIBS "FRAMEWORK_${COMPONENT}" rmId)
    IF(rmId LESS 0) #component is not in rmlist
	ADD_SUBDIRECTORY(${COMPONENT})
    ADD_DEPENDENCIES(FRAMEWORK_${COMPONENT} PLATFORM)
    SET_GLOBAL(FRAMEWORK_LIBS "${FRAMEWORK_LIBS};FRAMEWORK_${COMPONENT}")
    ENDIF()
    UNSET(COMPONENT_LIBRARY_NAME)
ENDFOREACH()
