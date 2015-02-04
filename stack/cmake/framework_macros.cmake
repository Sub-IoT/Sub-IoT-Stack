#This file contains helper MACRO's that are only available to code from the framework itself

# Override a specific component of the framework with alternative sources. 
# This MACRO is mainly intended to be used by individual platforms but can also be used by specific chip 
# implementations (useful for MCU's).
#
#
# Usage:
#    OVERRIDE_COMPONENT(component <source_file> <source_file> ...)
#
MACRO(OVERRIDE_COMPONENT component)
    SET_GLOBAL(FRAMEWORK_OVERRIDE_LIBS "${FRAMEWORK_OVERRIDE_LIBS};FRAMEWORK_${component}")
    ADD_LIBRARY(FRAMEWORK_${component} OBJECT ${ARGN})
ENDMACRO()

