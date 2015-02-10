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

#Each application should yield a single executable
ADD_EXECUTABLE(simple_leds app.c)

#Add the framework (and one or more modules) as libraries to the application
TARGET_LINK_LIBRARIES(simple_leds framework)

#Generate IDE specific binaries (if needed) 
GENERATE_SIMPLICITY_STUDIO_FILES(simple_leds)

