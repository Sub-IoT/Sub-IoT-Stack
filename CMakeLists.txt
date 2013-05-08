CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (D7AOSS C)

#######################################
# Modules
#######################################
ADD_SUBDIRECTORY(d7aoss)
ADD_SUBDIRECTORY(examples)

#######################################
# Configuration
#######################################
SET(PhyRadioDriver "CC430" CACHE STRING "Radio chip driver used by PHY")
SET_PROPERTY(CACHE PhyRadioDriver PROPERTY STRINGS CC430 STUB)
