INCLUDE_DIRECTORIES(${D7AOSS_SOURCE_DIR}/d7aoss/)
INCLUDE_DIRECTORIES(${D7AOSS_SOURCE_DIR}/d7aoss/framework)
INCLUDE_DIRECTORIES(${D7AOSS_SOURCE_DIR}/d7aoss/hal)
INCLUDE_DIRECTORIES(${D7AOSS_SOURCE_DIR}/d7aoss/dll)

ADD_EXECUTABLE(request_response_example_resp main.c filesystem.c)
TARGET_LINK_LIBRARIES(request_response_example_resp OSS7STACK)

IF(D7AOSS_HAL_DRIVER STREQUAL "efm32gg")
	SET_TARGET_PROPERTIES(request_response_example_resp PROPERTIES LINK_FLAGS "-T${D7AOSS_SOURCE_DIR}/d7aoss/hal/efm32gg/CMSIS/device/linker/efm32gg.ld")
ENDIF()
