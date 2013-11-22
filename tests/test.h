#ifndef TEST_H
#define TEST_H

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"
#include <unistd.h>

#include <assert.h>

// If unit testing is enabled override assert with mock_assert().
#define UNIT_TESTING // TODO define using cmake
#ifdef UNIT_TESTING
extern void mock_assert(const int result, const char* const expression,
                        const char * const file, const int line);
#undef assert
#define assert(expression) \
    mock_assert((int)(expression), #expression, __FILE__, __LINE__);
#endif // UNIT_TESTING

#endif // TEST_H
