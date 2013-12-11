#include "SimHalInterface.h"

#include "Oss7Test.h"


void set_timer(int t)
{
    oss7_node()->oss7_api_setTimer(t);
}

void timer_elapsed_callback()
{
    oss7_node()->oss7_api_timerEventCallback();
}

void log_msg(char *msg, uint8_t len)
{
    oss7_node()->oss7_api_trace(msg, len);
}

