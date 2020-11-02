/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define _GNU_SOURCE
#include <sched.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <debug.h>

#include "platform.h"
#include "button.h"
#include "hwgpio.h"
#include "hwatomic.h"
#include "scheduler.h"
#include "errors.h"


typedef struct
{
	ubutton_callback_t callback;
} button_info_t;

static char stack_top[8192];
static int button_handler(void *arg);

static button_info_t buttons[PLATFORM_NUM_BUTTONS];


__LINK_C void __ubutton_init()
{
	for (int i = 0; i < PLATFORM_NUM_BUTTONS; i++)
		buttons[i].callback = NULL;
	clone(button_handler, &stack_top[8192], CLONE_VM, NULL);
}

__LINK_C bool ubutton_pressed(button_id_t button_id)
{
	return false; // Polling not supported in this emulation
}

__LINK_C error_t ubutton_register_callback(button_id_t button_id, ubutton_callback_t callback)
{
	if(button_id >= PLATFORM_NUM_BUTTONS)
		return ESIZE;

	start_atomic();
	buttons[button_id].callback = callback;
	end_atomic();

	return SUCCESS;
}

__LINK_C error_t ubutton_deregister_callback(button_id_t button_id, ubutton_callback_t callback)
{
	return ubutton_register_callback(button_id, NULL);
}


static struct termios orig_termios;

static void restore_termio(void)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static int button_handler(void *arg)
{
	tcgetattr(STDIN_FILENO, &orig_termios);
	atexit(restore_termio);
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

	while (1)
	{
		int c;
		if (read(STDIN_FILENO, &c, 1) == 1)
		{
			uint8_t button_id = c - '0';
			if (button_id >= 0 && button_id < PLATFORM_NUM_BUTTONS &&
					buttons[button_id].callback)
			{
				buttons[button_id].callback(button_id);
			}
		}
	}

	return 0;
}
