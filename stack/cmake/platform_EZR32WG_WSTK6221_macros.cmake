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

#This file contains additional MACRO's that are only available for the 'gecko' platform

#Always include 'platform_default.cmake' so non-overriden macro's default to the 'default' implementation
include (${PROJECT_SOURCE_DIR}/cmake/platform_default_macros.cmake)

# Generate files needed to debug the application in Simplicity Studio from the files generated
# for the specified <target>. 
#
# Usage:
#    GENERATE_SIMPLICITY_STUDIO_FILES(target)
#	<target>	the <target> used to generate the 'main' executable. 
#    	
MACRO(GENERATE_SIMPLICITY_STUDIO_FILES target)
    SET_TARGET_PROPERTIES(${target} PROPERTIES LINK_FLAGS "-Xlinker -Map=${target}.mmp")
    ADD_CUSTOM_COMMAND(OUTPUT ${target}.hex
		   COMMAND ${CMAKE_OBJCOPY} -O ihex ${target} ${target}.hex DEPENDS ${target})
    ADD_CUSTOM_COMMAND(OUTPUT ${target}.bin
    		   COMMAND ${CMAKE_OBJCOPY} -O binary ${target} ${target}.bin DEPENDS ${target})
    ADD_CUSTOM_COMMAND(OUTPUT ${target}.map
		   COMMAND sh ${PROJECT_SOURCE_DIR}/tools/gcc-arm-embedded/fix_linker_map_paths.sh ${target}.mmp ${target}.map 
		   COMMAND ${CMAKE_COMMAND} -E remove -f ${target}.mmp
		   DEPENDS ${target})
    ADD_CUSTOM_COMMAND(OUTPUT ${target}.axf 
		   COMMAND ${CMAKE_COMMAND} -E copy ${target} ${target}.axf DEPENDS ${target})
    ADD_CUSTOM_TARGET(${target}_addn ALL DEPENDS ${target}.hex ${target}.bin ${target}.map ${target}.axf)
ENDMACRO()

