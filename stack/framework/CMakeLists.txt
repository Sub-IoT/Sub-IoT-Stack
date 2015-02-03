#clear a number of persistent variables (used to pass between different CMakeFiles)
UNSET(FRAMEWORK_OVERRIDE_LIBS CACHE)
UNSET(FRAMEWORK_LIBS CACHE)
UNSET(PLATFORM_CHIP_LIBS CACHE)

#GET_PROPERTY(__includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#MESSAGE(AUTHOR_WARNING "Includes@${CMAKE_CURRENT_SOURCE_DIR}: ${__includes}")

#Make sure that the framework interface is reacheable throughout the source tree
GLOBAL_INCLUDE_DIRECTORIES("inc")

#GET_PROPERTY(__includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#MESSAGE(AUTHOR_WARNING "Includes@${CMAKE_CURRENT_SOURCE_DIR}: ${__includes}")



#define the macro that can be used by the hal to override components
MACRO(OVERRIDE_COMPONENT component)
    SET(FRAMEWORK_OVERRIDE_LIBS "${FRAMEWORK_OVERRIDE_LIBS};FRAMEWORK_${component}" CACHE INTERNAL "")
    ADD_LIBRARY(FRAMEWORK_${component} OBJECT ${ARGN})
ENDMACRO()

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
