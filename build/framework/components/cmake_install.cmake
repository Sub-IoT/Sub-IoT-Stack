# Install script for directory: /media/sf_Virtualbox-shared/oss-7/stack/framework/components

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/aes/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/alp/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/cli/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/compress/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/console/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/crc/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/fec/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/fifo/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/fs/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/log/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/modem_interface/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/node_globals/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/phy/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/pn9/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/random/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/scheduler/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/segger_rtt/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/shell/cmake_install.cmake")
  include("/media/sf_Virtualbox-shared/oss-7/build/framework/components/timer/cmake_install.cmake")

endif()

