SET(CMOCKERY_SRCS cmockery/cmockery.c)
INCLUDE_DIRECTORIES(cmockery)
ADD_EXECUTABLE(timer_tests timer_tests.c ${CMOCKERY_SRCS})
TARGET_LINK_LIBRARIES(timer_tests D7AOSS rt)

ADD_EXECUTABLE(queue_tests queue_tests.c ${CMOCKERY_SRCS})
TARGET_LINK_LIBRARIES(queue_tests D7AOSS rt)
