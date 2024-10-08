# 
# OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
# lowpower wireless sensor communication
#
# Copyright 2019 Aloxy
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
# Toolchain setup gcc
#######################################

# -fshort enums is required as this seems to be default with gcc-arm-none-eabi and we do some static asserts using this assumption
# we also set these for c++ as this otherwise gives problems when linking mocks (C) against our test code (C++)

SET(CMAKE_C_FLAGS_DEBUG "-g -fshort-enums -Wno-typedef-redefinition -Wno-switch -Wno-gnu-variable-sized-type-not-at-end")
SET(CMAKE_CXX_FLAGS_DEBUG "-fshort-enums -g")
