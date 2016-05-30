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

	.section .text
	.global	setjmp
	.global	longjmp

setjmp:	
#if defined (__APS3__) || defined (__APS3B__)
	st  r1, [r2]
	stq r8, [r2]+1*4
	stq r12,[r2]+5*4
#else
	st  r1, [r2]+0*4
	st  r8, [r2]+1*4
	st  r9, [r2]+2*4
	st  r10,[r2]+3*4
	st  r11,[r2]+4*4
	st  r12,[r2]+5*4
	st  r13,[r2]+6*4
	st  r14,[r2]+7*4
	st  r15,[r2]+8*4
#endif	
	mov r2, #0
	jmp [r15]

longjmp:
#if defined (__APS3__) || defined (__APS3B__)
	ld  r1, [r2]
	cmp r3, #0
	ldq r8, [r2]+1*4
	ldq r12,[r2]+5*4
#else	
	ld  r1, [r2]+0*4
	cmp r3, #0
	ld  r8, [r2]+1*4
	ld  r9, [r2]+2*4
	ld  r10,[r2]+3*4
	ld  r11,[r2]+4*4
	ld  r12,[r2]+5*4
	ld  r13,[r2]+6*4
	ld  r14,[r2]+7*4
	ld  r15,[r2]+8*4
#endif	
	mov r2, r3
	bne .L1
	mov r2, #1
.L1:	jmp [r15]
