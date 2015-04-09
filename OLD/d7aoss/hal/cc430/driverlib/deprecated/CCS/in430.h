/*----------------------------------------------------------------------------*/
/* in430.h      - Intrinsic function prototypes and convenience mapping       */
/*                macros for migrating code from the IAR platform.            */
/*                                                                            */
/*  Ver | dd mmm yyyy | Who  | Description of changes                         */
/* =====|=============|======|=============================================   */
/*  0.01| 06 Apr 2004 | A.D. | First Prototype                                */
/*  0.02| 22 Jun 2004 | A.D. | File reformatted                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#ifndef __IN430_H
#define __IN430_H

/*----------------------------------------------------------------------------*/
/* COMPILER INTRINSIC FUNCTIONS                                               */
/*----------------------------------------------------------------------------*/

void _enable_interrupts(void); 
void _disable_interrupts(void); 
unsigned short _bic_SR_register(unsigned short mask);
unsigned short _bic_SR_register_on_exit(unsigned short mask); 
unsigned short _bis_SR_register(unsigned short mask); 
unsigned short _bis_SR_register_on_exit(unsigned short mask);
unsigned short _get_SR_register(void); 
unsigned short _get_SR_register_on_exit(void); 
unsigned short _swap_bytes(unsigned short src); 
void _nop(void); 
void _never_executed(void);

/*----------------------------------------------------------------------------*/
/* INTRINSIC MAPPING FOR IAR V1.XX                                            */
/*----------------------------------------------------------------------------*/

#define _EINT()                         _enable_interrupts()
#define _DINT()                         _disable_interrupts()
#define _BIC_SR(x)                      _bic_SR_register(x)
#define _BIC_SR_IRQ(x)                  _bic_SR_register_on_exit(x)
#define _BIS_SR(x)                      _bis_SR_register(x)
#define _BIS_SR_IRQ(x)                  _bis_SR_register_on_exit(x)
#define _SWAP_BYTES(x)                  _swap_bytes(x)
#define _NOP()                          _nop()

/*----------------------------------------------------------------------------*/
/* INTRINSIC MAPPING FOR IAR V2.XX/V3.XX                                      */
/*----------------------------------------------------------------------------*/

#define __enable_interrupt()            _enable_interrupts()
#define __disable_interrupt()           _disable_interrupts()
#define __bic_SR_register(x)            _bic_SR_register(x)
#define __bic_SR_register_on_exit(x)    _bic_SR_register_on_exit(x)
#define __bis_SR_register(x)            _bis_SR_register(x)
#define __bis_SR_register_on_exit(x)    _bis_SR_register_on_exit(x)
#define __get_SR_register()             _get_SR_register()
#define __get_SR_register_on_exit()     _get_SR_register_on_exit()
#define __swap_bytes(x)                 _swap_bytes(x)
#define __no_operation()                _nop()

#endif /* __IN430_H */
