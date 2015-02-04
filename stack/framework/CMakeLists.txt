#Export the interface of the HAL to the rest of the code base
GLOBAL_INCLUDE_DIRECTORIES("inc")

#Include framework-specific helper MACRO's
include(${CMAKE_SOURCE_DIR}/cmake/framework_macros.cmake)

#add the hal
ADD_SUBDIRECTORY("hal")
#add the non-hal components
ADD_SUBDIRECTORY("components")

SET(FRAMEWORK_SOURCES framework_bootstrap.c)

# Assemble the library
# PLATFORM_CHIP_LIBS: Object libraries of individual chips
# PLATFORM: Object library of the platform itself
# FRAMEWORK_OVERRIDE_LIBS: non-hal object libraries that were overridden by the HAL
# FRAMEWORK_LIBS: non-hal object libraries (not overridden)
SET(LIBNAMES ${PLATFORM_CHIP_LIBS} "PLATFORM" ${FRAMEWORK_OVERRIDE_LIBS} ${FRAMEWORK_LIBS})

#Some wizardry to get ADD_LIBRARY to accept everything
FOREACH(libname ${LIBNAMES})
    LIST(APPEND LIBS "$<TARGET_OBJECTS:${libname}>")
ENDFOREACH()
#AND finally add the library
ADD_LIBRARY(framework STATIC ${LIBS} ${FRAMEWORK_SOURCES})
