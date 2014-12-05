SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(../hal/)
ADD_LIBRARY(FRAMEWORK 
    log.c
    queue.c
    timer.c
    #timer_new.c
    ) 
