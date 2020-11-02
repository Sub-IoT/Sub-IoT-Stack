#include <string.h>
#include <stdio.h>

#include "framework_defs.h"

#ifdef FRAMEWORK_CONSOLE_ENABLED

#include "platform_defs.h"
#include "console.h"
#include "debug.h"

void console_init(void) {
}

void console_enable(void) {
}

void console_disable(void) {
}

inline void console_print_byte(uint8_t byte) {
	putc(byte);
}

inline void console_print_bytes(uint8_t* bytes, uint8_t length) {
	while (length--)
		console_print_byte(*bytes++);
}

inline void console_print(char* string) {
    console_print_bytes((uint8_t*) string, strlen(string));
}

#endif
