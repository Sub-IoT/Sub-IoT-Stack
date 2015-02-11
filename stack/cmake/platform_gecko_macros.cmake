#This file contains additional MACRO's that are only available for the 'gecko' platform

#Always include 'platform_default.cmake' so non-overriden macro's default to the 'default' implementation
include (${CMAKE_SOURCE_DIR}/cmake/platform_default_macros.cmake)

# Generate files needed to debug the application in Simplicity Studio from the files generated
# for the specified <target>. 
#
# Usage:
#    GENERATE_SIMPLICITY_STUDIO_FILES(target)
#	<target>	the <target> used to generate the 'main' executable. 
#    	
MACRO(GENERATE_SIMPLICITY_STUDIO_FILES target)
    SET_TARGET_PROPERTIES(${target} PROPERTIES LINK_FLAGS "-Xlinker -Map='${target}.mmp'")
    ADD_CUSTOM_COMMAND(OUTPUT ${target}.hex
		   COMMAND ${CMAKE_OBJCOPY} -O ihex ${target} ${target}.hex DEPENDS ${target})
    ADD_CUSTOM_COMMAND(OUTPUT ${target}.bin
    		   COMMAND ${CMAKE_OBJCOPY} -O binary ${target} ${target}.bin DEPENDS ${target})
    ADD_CUSTOM_COMMAND(OUTPUT ${target}.map
		   COMMAND ${CMAKE_SOURCE_DIR}/tools/gcc-arm-embedded/fix_linker_map_paths.sh ${target}.mmp ${target}.map 
		   COMMAND ${CMAKE_COMMAND} -E remove -f ${target}.mmp
		   DEPENDS ${target})
    ADD_CUSTOM_COMMAND(OUTPUT ${target}.axf 
		   COMMAND ${CMAKE_COMMAND} -E copy ${target} ${target}.axf DEPENDS ${target})
    ADD_CUSTOM_TARGET(${target}_addn ALL DEPENDS ${target}.hex ${target}.bin ${target}.map ${target}.axf)
ENDMACRO()

