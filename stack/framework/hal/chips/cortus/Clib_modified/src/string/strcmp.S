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

	.file	"strcmp.S"
	.section .text

#ifdef __APSHD__
	.balign	4
	.global	strcmp
	.type	strcmp, @function
strcmp:	
	or	r4, r2, r3
	and.cc	r4, #3   	;  !aligned ?
	movhi   r6, #high(0x01010101)
	add     r6, r6,  #low(0x01010101)
	bne	.Lbyte_loop		
	movhi   r7, #high(0x80808080)
	add     r7, r7,  #low(0x80808080)
	sub	r1, #4
	st	r8, [r1]
.Lword_loop:			; this label is word aligned!
	ld	r4, [r2]
	ld	r5, [r3]
	inv	r8, r4
	cmp	r4, r5
	sub	r4, r6
	and	r4, r8
	bne	.Lbyte_loop1
	and.cc	r4, r7
	add	r2, #4
	add	r3, #4
	beq     .Lword_loop
.Lwords_eq:	
	ld	r8, [r1]
	add	r1, #4
	mov	r2, r0
	jmp	[r15]

	.balign	4
.Lbyte_loop1:	
	ld	r8, [r1]
	add	r1, #4
.Lbyte_loop:		
	ldub.cc	r4, [r2]
	ldub    r5, [r3]
	beq.l	.Lnull
	
	sub.cc	r4, r5
	add	r2, #1
	add	r3, #1
	beq	.Lbyte_loop

.Lbyte_ne:mov	r2, r4
	jmp	[r15]

.Lnull:	sub	r2, r4, r5
	jmp	[r15]
#else
#if defined(__APS3__) || defined(__APS3B__) || defined(__APS__)
	.balign	4
	.global	strcmp
	.type	strcmp, @function
strcmp:	
	or	r4, r2, r3
	and.cc	r4, #3   	;  !aligned ?
	movhi   r6, #high(0x01010101)
	add     r6,  #low(0x01010101)
	bne	.Lbyte_loop		
	movhi   r7, #high(0x80808080)
	add     r7,  #low(0x80808080)
	sub	r1, #4
	st	r8, [r1]
.Lword_loop:			; this label is word aligned!
	ld	r4, [r2]
	ld	r5, [r3]
	inv	r8, r4
	cmp	r4, r5
	sub	r4, r6
	and	r4, r8
	bne	.Lbyte_loop1
	and.cc	r4, r7
	add	r2, #4
	add	r3, #4
	beq     .Lword_loop
.Lwords_eq:	
	ld	r8, [r1]
	add	r1, #4
	mov	r2, r0
	jmp	[r15]

	.balign	4
.Lbyte_loop1:	
	ld	r8, [r1]
	add	r1, #4
.Lbyte_loop:		
	ldub.cc	r4, [r2]
	ldub    r5, [r3]
	beq.l	.Lnull
	
	sub.cc	r4, r5
	add	r2, #1
	add	r3, #1
	beq	.Lbyte_loop

.Lbyte_ne:mov	r2, r4
	jmp	[r15]

.Lnull:	sub	r2, r4, r5
	jmp	[r15]
#else
#ifdef __APS2__
	.global	strcmp
	.type	strcmp, @function
strcmp:	
	or	r4, r2, r3
	and.cc	r4, r4, #3   	;  !aligned ?
	movhi   r6, #high(0x01010101)
	add     r6, r6, #low(0x01010101)
	bne	.Lbyte_loop		
	movhi   r7, #high(0x80808080)
	add     r7, r7, #low(0x80808080)
	sub	r1, r1, #4
	st	r8, [r1]+0
.Lword_loop:			; this label is word aligned!
	ld	r4, [r2]+0
	ld	r5, [r3]+0
	xnor 	r8, r0, r4
	cmp	r4, r5
	sub	r4, r4, r6
	and	r4, r4, r8
	bne	.Lbyte_loop1
	and.cc	r4, r4, r7
	add	r2, r2, #4
	add	r3, r3, #4
	beq     .Lword_loop
.Lwords_eq:	
	ld	r8, [r1]+0
	add	r1, r1, #4
	mov	r2, r0
	jmp	[r15]

	.balign	4
.Lbyte_loop1:	
	ld	r8, [r1]+0
	add	r1, r1, #4
.Lbyte_loop:		
	ldub.cc	r4, [r2]+0
	ldub    r5, [r3]+0
	beq	.Lnull
	
	sub.cc	r4, r4, r5
	add	r2, r2, #1
	add	r3, r3, #1
	beq	.Lbyte_loop

.Lbyte_ne:mov	r2, r4
	jmp	[r15]

.Lnull:	sub	r2, r4, r5
	jmp	[r15]

#else
#ifdef __APS16__
	.balign	4
	.global	strcmp
	.type	strcmp, @function
strcmp:	
	mov	r4, r2
	or	r4, r3
	and.cc	r4, #1   	;  !aligned ?
	mov     r6, #high(0x0101)
	bne	byte_loop		
	mov     r7,  #low(0x8080)
	sub	r1, #2
	st	r8, [r1]
word_loop:			; this label is word aligned!
	ld	r4, [r2]
	ld	r5, [r3]
	inv	r8, r4
	cmp	r4, r5
	sub	r4, r6
	and	r4, r8
	bne	byte_loop1
	and.cc	r4, r7
	add	r2, #2
	add	r3, #2
	beq     word_loop
words_eq:	
	ld	r8, [r1]
	add	r1, #2
	mov	r2, r0
	jmp	[r15]

	.balign	4
byte_loop1:	
	ld	r8, [r1]
	add	r1, #2
byte_loop:		
	ldub.cc	r4, [r2]
	ldub    r5, [r3]
	beq	null
	
	sub.cc	r4, r5
	add	r2, #1
	add	r3, #1
	beq	byte_loop

byte_ne:mov	r2, r4
	jmp	[r15]

null:	sub	r4, r5
	mov	r2, r4
	jmp	[r15]
#else
#error "Invalid target"		
#endif
#endif
#endif	
#endif	
