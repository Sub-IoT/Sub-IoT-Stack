
ADD_EXECUTABLE(simple_leds app.c)
TARGET_LINK_LIBRARIES(simple_leds framework)

GENERATE_SIMPLICITY_STUDIO_FILES(simple_leds)

