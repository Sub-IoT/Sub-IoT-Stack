#By convention, application parameters should be prefixed with '${APP_PREFIX}'
#APP_OPTION(${APP_PREFIX}_gui "Build the GUI" OFF)
#APP_PARAM(${APP_PREFIX}_default_config ".config" FILEPATH "Default Configuration File")
#APP_PARAM(${APP_PREFIX}_endianness "big_endian" STRING "Endianness of the application")
#SET_PROPERTY(CACHE ${APP_PREFIX}_endianness PROPERTY STRINGS "big_endian;little_endian")
#GLOBAL_INCLUDE_DIRECTORIES("Hello")
#GLOBAL_ADD_DEFINITIONS("-DHello=WORLD")

ADD_EXECUTABLE(simple_leds app.c)
TARGET_LINK_LIBRARIES(simple_leds framework)

GENERATE_SIMPLICITY_STUDIO_FILES(simple_leds)

#SET_TARGET_PROPERTIES(simple_leds PROPERTIES LINK_FLAGS "-Xlinker -Map='simple_leds.mmp'")
#SET_TARGET_PROPERTIES(simple_leds PROPERTIES LINK_LIBRARIES "-Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group")
#ADD_CUSTOM_COMMAND(OUTPUT simple_leds.hex
#		   COMMAND ${CMAKE_OBJCOPY} -O ihex simple_leds simple_leds.hex DEPENDS simple_leds)
#ADD_CUSTOM_COMMAND(OUTPUT simple_leds.bin
#		   COMMAND ${CMAKE_OBJCOPY} -O binary simple_leds simple_leds.bin DEPENDS simple_leds)
#ADD_CUSTOM_COMMAND(OUTPUT simple_leds.map
#		   COMMAND ${CMAKE_SOURCE_DIR}/tools/gcc-arm-embedded/fix_linker_map_paths.sh simple_leds.mmp simple_leds.map DEPENDS simple_leds)
#ADD_CUSTOM_COMMAND(OUTPUT simple_leds.axf 
#		   COMMAND ${CMAKE_COMMAND} -E copy simple_leds simple_leds.axf DEPENDS simple_leds)
#ADD_CUSTOM_COMMAND(OUTPUT simple_leds.elf 
#		   COMMAND ${CMAKE_COMMAND} -E copy simple_leds simple_leds.elf DEPENDS simple_leds)
#		   
#ADD_CUSTOM_TARGET(simple_leds_addn ALL DEPENDS simple_leds.hex simple_leds.bin simple_leds.map simple_leds.axf simple_leds.elf)
		   
#ADD_CUSTOM_COMMAND(OUTPUT scheduler_example.bin
#		   COMMAND ${CMAKE_OBJDUMP} -binary scheduler_example.axf scheduler_example.bin
#		   DEPENDS scheduler_example)
#ENDIF()



