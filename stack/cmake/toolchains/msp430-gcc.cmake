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

#######################################
# Toolchain setup msp430-gcc
#######################################

include(CMakeForceCompiler)

CMAKE_FORCE_C_COMPILER(msp430-elf-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(msp430-elf-g++ GNU)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_CROSSCOMPILING 1)


SET(TOOLCHAIN_DIR "" CACHE PATH "The directory containing all the cross compilation tools. (Compilation will fail if this is not set correctly)")

# where is the target environment 
LIST(APPEND CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}")

MESSAGE(STATUS "Cross-compiling using msp430-gcc toolchain")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(MSP430_SUPPORT_FILES "${TOOLCHAIN_DIR}/include/")
