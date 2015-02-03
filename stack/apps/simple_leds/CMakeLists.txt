#By convention, application parameters should be prefixed with '${APP_PREFIX}'
#APP_OPTION(${APP_PREFIX}_gui "Build the GUI" OFF)
#APP_PARAM(${APP_PREFIX}_default_config ".config" FILEPATH "Default Configuration File")
#APP_PARAM(${APP_PREFIX}_endianness "big_endian" STRING "Endianness of the application")
#SET_PROPERTY(CACHE ${APP_PREFIX}_endianness PROPERTY STRINGS "big_endian;little_endian")
#GLOBAL_INCLUDE_DIRECTORIES("Hello")
#GLOBAL_ADD_DEFINITIONS("-DHello=WORLD")

ADD_EXECUTABLE(simple_leds app.c)
TARGET_LINK_LIBRARIES(simple_leds framework)


