#ifndef SIMHALINTERFACE_H
#define SIMHALINTERFACE_H

#include <stdint.h>

// callable from OSS7 stack
#ifdef __cplusplus
extern "C" {
#endif

void set_timer(int t);
void timer_elapsed_callback();
void log_msg(char* msg, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif // SIMHALINTERFACE_H
