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

#Each Framework component must generate a single OBJECT library named
#'${COMPONENT_LIBRARY_NAME}'
ADD_LIBRARY(${COMPONENT_LIBRARY_NAME} OBJECT aes.c ccm.c)

GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
TARGET_INCLUDE_DIRECTORIES(${COMPONENT_LIBRARY_NAME} PUBLIC ${__global_include_dirs})
