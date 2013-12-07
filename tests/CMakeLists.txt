SET(CMOCKERY_SRCS cmockery/cmockery.c)
INCLUDE_DIRECTORIES(cmockery)
#ADD_EXECUTABLE(timer_tests timer_tests.h ${CMOCKERY_SRCS})
#TARGET_LINK_LIBRARIES(timer_tests D7AOSS rt)

ADD_EXECUTABLE(test_runner test_runner.c timer_tests.h queue_tests.h ${CMOCKERY_SRCS})
TARGET_LINK_LIBRARIES(test_runner D7AOSS rt)
SET_TARGET_PROPERTIES(test_runner PROPERTIES LINKER_LANGUAGE C)
