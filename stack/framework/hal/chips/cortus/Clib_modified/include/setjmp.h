/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.
 *
 *             (C) Copyright 2004, 2005, 2006 Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement 
 * from Cortus S.A. (http://www.cortus.com)
 *
 * $CortusRelease$
 * $FileName$
 *
 *********************************************************************************/

#ifndef _SETJMP_H_
#define _SETJMP_H_

typedef unsigned jmp_buf[10];

void longjmp (jmp_buf __jmpb, int __retval);
int  setjmp (jmp_buf __jmpb);

#endif /* _SETJMP_H_ */

