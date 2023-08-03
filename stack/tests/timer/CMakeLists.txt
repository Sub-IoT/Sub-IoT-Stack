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
project(test_timer)
cmake_minimum_required(VERSION 2.8)

add_subdirectory(./mocks)

#create a lib out of the scheduler
add_library(scheduler STATIC ../../framework/components/scheduler/scheduler.c)
set_target_properties(scheduler PROPERTIES LINKER_LANGUAGE C)
target_include_directories(scheduler PUBLIC ../../framework/inc ../../framework/hal/inc ./) #to include framework_defs.h

add_executable(${PROJECT_NAME} main.c ) #the main includes timer.c directly

target_include_directories(${PROJECT_NAME} PUBLIC ../../framework/components/timer ../../framework/inc ../../framework/hal/inc ./) 
target_link_libraries (${PROJECT_NAME} scheduler mocks)