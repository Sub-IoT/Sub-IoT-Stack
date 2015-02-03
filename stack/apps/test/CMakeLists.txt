APP_PARAM(${APP_PREFIX}_config "The_APP_Config" STRING "App Configuration")

ADD_EXECUTABLE(bla bla.c)

ADD_DEFINITIONS("-DAPP_CONFIG=${${APP_PREFIX}_config}")

TARGET_LINK_LIBRARIES(bla framework)