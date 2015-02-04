#######################################
# Toolchain setup
#######################################
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER msp430-elf-gcc)
SET(CMAKE_LINKER msp430-elf-gcc)
#SET(CMAKE_AR msp430-elf-ar)
#SET(CMAKE_RANLIB msp430-elf-ranlib)
#include(CMakeForceCompiler)
#CMAKE_FORCE_C_COMPILER(msp430-elf-gcc GNU)

#SET(CMAKE_FIND_ROOT_PATH  /home/glenn/bin/ti/ccsv6/tools/compiler/gcc_msp430_4.8.371/)
#SET(MSP430_SUPPORT_FILES /home/glenn/bin/ti/ccsv6/ccs_base/msp430/include_gcc/)
SET(CMAKE_FIND_ROOT_PATH /opt/toolchains/ti/gcc/)
SET(MSP430_SUPPORT_FILES /opt/toolchains/ti/gcc/include/)
# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)