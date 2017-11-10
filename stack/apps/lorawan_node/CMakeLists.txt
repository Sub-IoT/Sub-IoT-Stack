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

#See the explanation of APP_OPTION and APP_PARAM in cmake/app_macros.cmake 
#for details on how to add application-specific CMake GUI entries

#By convention, application parameters should be prefixed with '${APP_PREFIX}'
#Some examples:
#APP_OPTION(${APP_PREFIX}_<option_name> "Option explanation" <default_value>)
#APP_PARAM(${APP_PREFIX}_<param_name> "<default_value>" <type> "Parameter explanation")
#
#Cache properties can be set on application parameters just like on regular cache parameters
#SET_PROPERTY(CACHE ${APP_PREFIX}_<param_name> PROPERTY STRINGS "value1;value2")
#

APP_BUILD(NAME ${APP_NAME} SOURCES lorawan_node.c LIBS lorawan framework)
