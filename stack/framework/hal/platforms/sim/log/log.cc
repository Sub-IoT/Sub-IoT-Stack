/*! \file log.c
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 * \author daniel.vandenakker@uantwerpen.be
 */

#include "log.h"
#include "ng.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "framework_defs.h"
#include "HalModule.h"
#include <iomanip>

#ifdef FRAMEWORK_LOG_ENABLED


static uint32_t NGDEF(counter);

extern "C" void log_counter_reset()
{
	NG(counter) = 0;
}

extern "C" void print_log_string(char* format, va_list args)
{
	if(!HalModule::getActiveModule()->traceEnabled())
	{
		//don't bother evaluating the parameters if traces are not enabled
		NG(counter)++;
	}
	else
	{
		char* ptr = 0x0;
		vasprintf(&ptr,format,args);
		if(ptr != 0x0)
		{
			(HalModule::getActiveModule()->get_log_stream())
				<< "Node " << get_node_global_id() << ": " << setfill('0') << setw(3) << NG(counter)++ << " " << ptr << std::endl;
			free(ptr);
		}
	}
}


extern "C" void log_print_string(char* format, ...)
{
	va_list args;
	va_start(args, format);
	print_log_string(format, args);
	va_end(args);
}

extern "C" void log_print_stack_string(char type, char* format, ...)
{
	va_list args;
	va_start(args, format);
	print_log_string(format, args);
	va_end(args);
}

extern "C" void log_print_data(uint8_t* message, uint32_t length)
{
	if(!HalModule::getActiveModule()->traceEnabled())
	{
		//don't bother evaluating the parameters if traces are not enabled
		NG(counter)++;
	}
	else
	{
		std::ostream& stream((HalModule::getActiveModule()->get_log_stream()));

		stream << "Node " << get_node_global_id() << ": " << setfill('0') << setw(3) << NG(counter)++;
		for( uint32_t i=0 ; i<length ; i++ )
		{
			stream << " " << setfill('0') << setw(2) << message[i];
		}
		stream << std::endl;
	}
}


#endif //FRAMEWORK_LOG_ENABLED
