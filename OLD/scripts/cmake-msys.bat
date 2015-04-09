cd ../oss7-build
cmake ../ -DD7AOSS_PHY_RADIO_DRIVER="cc430" -DD7AOSS_HAL_DRIVER="cc430" -DD7AOSS_BUILD_EXAMPLES=y -DCMAKE_TOOLCHAIN_FILE="../cmake/toolchains/mspgcc.cmake" -G "MSYS Makefiles"
cd ../scripts