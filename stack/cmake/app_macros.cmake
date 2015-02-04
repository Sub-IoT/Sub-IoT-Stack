#This file contains helper MACRO's that are only available to individual applications

# Declare a parameter for the application. Application parameters are 
# automaticall shown/hidden in the CMake GUI depending on whether or 
# not the application is enabled.
# 
# Application parameters are stored in the cache, just like regular parameters.
# This means that the properties of the parameter can be set in the normal manner
# (by using the SET_PROPERTY command)
#
# Usage:
#    APP_PARAM(<var> <default_value> <type> <doc_string>)
#	<var> 		is the name of the variable to set. By convention 
#			all application parameters should be prefixed by the 
#			${APP_PREFIX} variable (see examples below).
#
#       <default_value> the default value of the parameter.
#
#	<type>		the type of the parameter. Any valid CMake cache <type>
#			except INTERNAL. At the time of writing these are the valid
#			types:
#				FILEPATH = File chooser dialog.
#				PATH     = Directory chooser dialog.
#				STRING   = Arbitrary string.
#				BOOL     = Boolean ON/OFF checkbox.
#				See http://www.cmake.org/cmake/help/v3.0/command/set.html
#
#	<doc_string>	the documentation string explaining the parameter
#
# Examples:
#	APP_PARAM(${APP_PREFIX}_STR "Default_Str" STRING "Parameter Explanation")
#		-Adds a STRING parameter '${APP_PREFIX}_STR' with default 
#		 value 'Default_Str' and explanation 'Parameter Explanation'
#		 The value of the parameter can be queried using:
#			${${APP_PREFIX}_STR}
#	
#	APP_PARAM(${APP_PREFIX}_LIST "item1" STRING "A List of items")
#	SET_PROPERTY(CACHE ${APP_PREFIX}_LIST PROPERTY STRINGS "item1;item2;item3")
#		-Adds a parameter '${APP_PREFIX}_LIST', with possible values
#		 'item1', 'item2' and 'item3'. Please note that this is done in
#		 exactly the same manner as for regular 'CACHE' parameters
MACRO(APP_PARAM var value type doc)
    SET(APP_PARAM_LIST "${APP_PARAM_LIST};${var}" CACHE INTERNAL "")
    #Set entry in cache if the application is being loaded
    SETCACHE_IF(${var} ${value} ${type} ${doc} ${APP_PREFIX})
ENDMACRO()

# Declare an option for the used application. Like application parameters,
# application options are automatically shown/hidden in the CMake GUI 
# depending on whether or not the application is enabled.
#
# As with application parameters, options are stored in the cache (just like
# regular CMake options)
#
# Usage:
#    APP_OPTION(<option> <doc_string> <default_value>)
#	<option>	is the name of the option to set. By convention 
#			all application options should be prefixed by the 
#			${APP_PREFIX} variable.
#
#	<doc_string>	the documentation string explaining the option
#
#       <default_value> the default value of the parameter.
#
#    In accordance with CMake's implementation of 'ADD_OPTION',
#    calling: 
#	APP_OPTION(<option> <doc_string> <default_value>)
#    is equivalent to calling:
#	APP_PARAM( <option> <default_value> BOOL <doc_string>
#
# Examples:
#	APP_OPTION(${APP_PREFIX}_OPT "OPT Explanation" FALSE)
#		-Adds a application option '{APP_PREFIX}_OPT' with 
#		 default value FALSE and explanation 'OPT Explanation'.
#		 The value of the option can be queried using:
#		    ${${APP_PREFIX}_OPT}
#
MACRO(APP_OPTION option doc default)
    APP_PARAM( ${option} ${default} BOOL ${doc})
ENDMACRO()

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





