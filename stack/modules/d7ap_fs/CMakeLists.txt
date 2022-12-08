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

# Module specific parameters and options can be defined using
# MODULE_OPTION and MODULE_PARAMETER
# See cmake/module_macros.cmake for more information

INCLUDE_DIRECTORIES(inc)

MODULE_OPTION(${MODULE_PREFIX}_USE_DEFAULT_SYSTEMFILES "Use the default D7AP systemfiles values" TRUE)
MODULE_OPTION(${MODULE_PREFIX}_DISABLE_PERMISSIONS "Temporary disable permission checks for testing purposes" FALSE)

MODULE_PARAM(${MODULE_PREFIX}_FILE_SIZE_MAX "77"  STRING "The default buffer size for file operations" )
MODULE_HEADER_DEFINE(
    BOOL ${MODULE_PREFIX}_USE_DEFAULT_SYSTEMFILES
    ${MODULE_PREFIX}_DISABLE_PERMISSIONS
    NUMBER ${MODULE_PREFIX}_FILE_SIZE_MAX)


#Generate the 'module_defs.h'
MODULE_BUILD_SETTINGS_FILE()

#Export the module-specific header files to the application by using
EXPORT_GLOBAL_INCLUDE_DIRECTORIES(.)

#By convention, each module should generate a single 'static' library that can be included by the application
ADD_LIBRARY(d7ap_fs STATIC
    d7ap_fs.c
    ../../fs/d7ap_fs_data.c
)

GET_PROPERTY(__global_include_dirs GLOBAL PROPERTY GLOBAL_INCLUDE_DIRECTORIES)
target_include_directories(d7ap_fs PUBLIC
	${__global_include_dirs}
    ${CMAKE_CURRENT_BINARY_DIR} # MODULE_D7AP_FS_defs.h
    inc
)

