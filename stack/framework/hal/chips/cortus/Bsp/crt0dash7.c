/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.S
 *
 *                 (C) Copyright 2006-2015 Cortus S.A.S
 *                         ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement 
 * from Cortus S.A.S (http://www.cortus.com)
 *
 * $CortusRelease$
 * $FileName$
 *
 * This file generates crt0.o and gcrt0.o (when PROFILING defined)
 *********************************************************************************/
#pragma GCC optimize ("Os,no-lto")

#include <stdio.h>
#include <stdlib.h>
#include <machine/cpu.h>
#include <machine/ic.h>
#include <machine/uart.h>
#include <machine/counter.h>
#include <machine/dcache.h>
#include <unistd.h>
#include <sys/gmon.h>
#include <machine/trace_buf.h>

/* Debugger printf buffer size.
   Define this to be bigger than 1 in order to enable it in place of uart output
   for stdio - should not be bigger than 255 */
#define DBG_PRINTF_BUF_SIZE 0

/* Clock frequency in KHZ
   Nominal clock frequency for a 90nm implementation is 400,000 KHZ - i.e. 400 MHZ.
   This is global so that it can be set/read by user code. Note that for
   profiling, it has to be set before profiling is started.  */
unsigned _cpu_clock_frequency_khz __attribute__((weak)) = 400000;

/* This is used by at_exit */
void* __dso_handle __attribute__((weak));

/********************************************************************************
 * Trap vector table
 ********************************************************************************/

/* Prototypes for trap handlers.
 * By defining these as weak, we allow the user to override these in his code */
void start (void);
void critical_error_handler      (void) __attribute__((weak));
void illegal_instruction_handler (void) __attribute__((weak));
void debugger_breakpoint_handler (void);
void debugger_stop_handler       (void);
void profile_handler             (void);
void interrupt5_handler  (void) __attribute__((weak));
void interrupt6_handler  (void) __attribute__((weak));
void interrupt7_handler  (void) __attribute__((weak));
void interrupt8_handler  (void) __attribute__((weak));
void interrupt9_handler  (void) __attribute__((weak));
void interrupt10_handler (void) __attribute__((weak));
void interrupt11_handler (void) __attribute__((weak));
void interrupt12_handler (void) __attribute__((weak));
void interrupt13_handler (void) __attribute__((weak));
void interrupt14_handler (void) __attribute__((weak));
void interrupt15_handler (void) __attribute__((weak));
void interrupt16_handler (void) __attribute__((weak));
void interrupt17_handler (void) __attribute__((weak));
void interrupt18_handler (void) __attribute__((weak));
void interrupt19_handler (void) __attribute__((weak));
void interrupt20_handler (void) __attribute__((weak));
void interrupt21_handler (void) __attribute__((weak));
void interrupt22_handler (void) __attribute__((weak));
void interrupt23_handler (void) __attribute__((weak));
void interrupt24_handler (void) __attribute__((weak));
void interrupt25_handler (void) __attribute__((weak));
void interrupt26_handler (void) __attribute__((weak));
void interrupt27_handler (void) __attribute__((weak));
void interrupt28_handler (void) __attribute__((weak));
void interrupt29_handler (void) __attribute__((weak));
void interrupt30_handler (void) __attribute__((weak));
void interrupt31_handler (void) __attribute__((weak));

volatile trap_handler_fp trap_vectors[32]
  __attribute ((section (".vectors"), used)) = 
{
    [0]  = start,
    [1]  = critical_error_handler,
    [2]  = illegal_instruction_handler,
    [3]  = debugger_breakpoint_handler,
    [4]  = debugger_stop_handler,
    [5]  = interrupt5_handler,
    [6]  = interrupt6_handler,
#ifdef PROFILING
    [7]  = profile_handler,
#else
    [7]  = interrupt7_handler,
#endif
    [8]  = interrupt8_handler,
    [9]  = interrupt9_handler,
    [10] = interrupt10_handler,
    [11] = interrupt11_handler,
    [12] = interrupt12_handler,
    [13] = interrupt13_handler,
    [14] = interrupt14_handler,
    [15] = interrupt15_handler,
    [16] = interrupt16_handler,
    [17] = interrupt17_handler,
    [18] = interrupt18_handler,
    [19] = interrupt19_handler,
    [20] = interrupt20_handler,
    [21] = interrupt21_handler,
    [22] = interrupt22_handler,
    [23] = interrupt23_handler,
    [24] = interrupt24_handler,
    [25] = interrupt25_handler,
    [26] = interrupt26_handler,
    [27] = interrupt27_handler,
    [28] = interrupt28_handler,
    [29] = interrupt29_handler,
    [30] = interrupt30_handler,
    [31] = interrupt31_handler,
};

/********************************************************************************
 * stdio
 * 
 * This is where all the libgloss IO functions end up.
 * These can be overriden in user supplied code to do something more interesting
 * and maybe even look at the FILE* parameter.
 *
 ********************************************************************************/

/* Register save area and status info for debugger */
typedef struct dbg_info_t {
    unsigned char status0, status1, status2, status3;
    unsigned r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
    unsigned r12, r13, r14, r15, rtt, psr;
#if DBG_PRINTF_BUF_SIZE > 1
    unsigned char size;
    unsigned char wrp;
    volatile unsigned char rdp;
    unsigned char buf[DBG_PRINTF_BUF_SIZE];
#endif
} dbg_info_t;
dbg_info_t _dbg_info __attribute__((section(".sbss.debugger_register_save_area")));

void uart1_outch_raw (int c) __attribute__((weak));

#if DBG_PRINTF_BUF_SIZE > 1
/* Output via debugger instead of uart */
void uart1_outch_raw (int c)
{
    /* Next location to write to */
    unsigned nwrp = _dbg_info.wrp + 1;

    /* Wrap around */
    if (nwrp == DBG_PRINTF_BUF_SIZE) nwrp = 0; 

    /* Wait if buffer is full (always one slot empty to detect difference
       between full and empty) */
    while (nwrp == _dbg_info.rdp - 1 || nwrp == _dbg_info.rdp + DBG_PRINTF_BUF_SIZE - 1);

    /* Put data in buffer */
    _dbg_info.buf[_dbg_info.wrp] = c;

    /* Update write pointer */
    _dbg_info.wrp = nwrp;

    /* Provide buffer size */
    _dbg_info.size = DBG_PRINTF_BUF_SIZE;
    
    /* Tell debugger to take a look */
    _dbg_info.status1 = 1;
    *(unsigned*)(0x50000000) = 0x10000;
}
#else
void uart1_outch_raw (int c)
{
    while (! (uart1->tx_status & 1)) ;
    uart1->tx_data = c;
}
#endif

void uart1_outch (void* f, int c) __attribute__((weak));
void uart1_outch (void* f, int c)
{
    if (c == '\n')
        uart1_outch_raw ('\r');
    uart1_outch_raw (c);
}

int uart1_inch (void* f) __attribute__((weak));
int uart1_inch (void* f)
{
    int c;
    while (! (uart1->rx_status & 1)) ;
    c = uart1->rx_data;
    return c;
}

/********************************************************************************
 * Stack and Heap are defined by the default linker script in .bss and will be 
 * initialized by the startup code below.
 * 
 * The sizes can be overriden by defining the symbols _STACK_SIZE and _HEAP_SIZE.
 * This can be done by using the macros in sys/memsize.h or by defining them
 * on the linker command line with --defsym.
 *
 * Also the maximum size of memory can be defined.
 *
 * e.g.
 *      HEAP_SIZE (16*1024);
 *      STACK_SIZE (4*1024);
 *      DATA_MEMORY_SIZE (64*1024);
 *      PROGRAM_MEMORY_SIZE (128*1024);
*/

/*
 * If stack overflow checking is enabled, set a break point on __stack_overflowed
 * in the debugger. At this stage a stack trace can be done. After that the 
 * stack is reset, and the function __stack_overflow is called. The default version
 * of this functions just does exit(3).
 */

/********************************************************************************
 * HW Initialization
 * e.g. set up uart baud rate
 ********************************************************************************/

/* For debugger stub, restore previous cache status */
int p_cache_en = 0;

/* All the peripherals etc should be fully initialised here in order to ensure a 
   successful reboot in the event of an abort () call or watch dog timeout. */
void initialise_hw (void) __attribute__((weak));
void initialise_hw (void)
{
#if DBG_PRINTF_BUF_SIZE > 1
    /* Intialise pointers */
    _dbg_info.wrp = 0;
    _dbg_info.rdp = 0;
    _dbg_info.size = DBG_PRINTF_BUF_SIZE;
#endif
    /* Enable the data cache (if present) */
    dcache->enable = 1;

    // Initialize the UART to reasonable values
    uart1->config = 0;
    uart1->selclk = 0;
    // Uart speed is 115200 bauds
    uart1->divider = 8*(50000000/115200);      
    uart1->enable = 1;
}

/********************************************************************************
 * Execution starts here (linker entry point)
 ********************************************************************************/

void start(void) __attribute__((section (".init"), used, noreturn, naked));

/* Type definition for a C++ static constructor function */
typedef void (*constructor_t)(void);

/* Static constructor array */
extern const constructor_t __CTOR_LIST__[], __CTOR_END__[];

/* Location of data for initialization of .sdata and .data sections */
extern const unsigned _data_image[];

/* Start of ram, Start/end of .sbss, .sdata/.data, .bss */
extern unsigned _sram[], _sbss[], _esbss[], _sdata[], _edata[], _bss[], _ebss[];

int main (void);
void init_profile (void);
void start_profile (void);
void stop_profile (void);

/* This function must be compiled with optimization enabled so that is does not
   place anything on the stack. The initialization of the .bss section will
   clear the stack resetting the variables used! */
void start(void)
{
    /* Initialize stack pointer */
    __asm__ ("  movhi   r1, #high(__stack_top)\n"
         "  add     r1, #low(__stack_top)\n");

    /* Initialize pointer to rodata */
    __asm__ ("  movhi   r8, #high(_rodata)\n"
         "  add     r8, #low(_rodata)\n");

    /* Initialize HW and memory etc and call main */
    while (1) {
        
        const constructor_t* ctor;
        const unsigned *src;
        unsigned *dst;
        int status;
        
        icache_flush ();
        initialise_hw ();
        
        /* Enable jtag interrupt from debugger. 
           This is what happens when you hit the stop button in the
           debugger GUI */
        irq[IRQ_DEBUGGER_STOP].ien = 1;
        
        /* Enable all interrupts */
        ic->ien = 1;
        cpu_int_enable ();
        
        /* Initialize the area at address 0 up to the register save area to
           something recognisable in the debugger to help track down NULL
           pointer deferencing. Depending on the linker script, this is usually 
           32 bytes of memory */
        for (dst = _sram; dst < (unsigned*) &_dbg_info; dst++)
            *dst = 0xa5a5a5a5;
        
        /* Initialize .sbss section */
        for (dst = _sbss; dst < _esbss; dst++) 
            *dst = 0;
        
        /* Initialize .sdata and .data sections */
        for (src = _data_image, dst = _sdata; dst < _edata;) 
            *dst++ = *src++;
        
        /* Initialize .bss section (including stack - make sure that
           'dst' is not on the stack! */
        for (dst = _bss; dst < _ebss; dst++) 
            *dst = 0;

        /* Call static constructors */
        for (ctor = __CTOR_LIST__; ctor < __CTOR_END__; ctor++)
            (**ctor)();
        
        /* Start profiling */
#ifdef PROFILING
        init_profile();

        /* The start can be anywhere convenient */
        start_profile();
#endif  
        /* Call main */
        status = main ();

        /* Exit should call destructors if desired */
        exit (status);
    }
}

/********************************************************************************
 * Monitor stub for debugger
 ********************************************************************************/
__asm__ (".section .text\n"
     "illegal_instruction_handler:\n"
     "   st  r2, [r0]+short(_dbg_info+2*4)\n"
     "   mov r2, #2\n"
     "   bra monitor\n"
     "debugger_breakpoint_handler:\n"
     "   st  r2, [r0]+short(_dbg_info+2*4)\n"
     "   mov r2, #3\n"
     "   bra monitor\n"
     "debugger_stop_handler:\n"
     "   st  r2, [r0]+short(_dbg_info+2*4)\n"
     "   mov r2, #4\n"
     "   bra monitor\n"
     "critical_error_handler:\n"
     "interrupt5_handler:\n"
     "interrupt6_handler:\n"
     "interrupt7_handler:\n"
     "interrupt8_handler:\n"
     "interrupt9_handler:\n"
     "interrupt10_handler:\n"
     "interrupt11_handler:\n"
     "interrupt12_handler:\n"
     "interrupt13_handler:\n"
     "interrupt14_handler:\n"
     "interrupt15_handler:\n"
     "uninitialized_trap_handler:\n"
     "   st  r2, [r0]+short(_dbg_info+2*4)\n"
     "   mov r2, #1\n"
     "   bra monitor\n"
     "\n"
     "; Monitor entry and exit - save and restore registers around call to monitor proper\n"
     "; save all registers\n"
     ".section .text\n"
     "monitor:\n"
     "   st  r1, [r0]+short(_dbg_info+1*4)\n"
     "   st  r3, [r0]+short(_dbg_info+3*4)\n"
     "   movhi r3, #high(0x50000000)\n"
     "; stop trace\n"
     "   st  r0, [r3]+0x1000\n"
     "   stq r4, [r0]+short(_dbg_info+4*4)\n"
     "   stq r8, [r0]+short(_dbg_info+8*4)\n"
     "   stq r12,[r0]+short(_dbg_info+12*4)\n"
     "   mov r4, rtt\n"
     "   mov r5, psr\n"
     "   std r4, [r0]+short(_dbg_info+16*4)\n"
     "\n"
     "; Save interrupt source information\n"
     "   stb  r2, [r0]+short(_dbg_info+0*4)\n"
     "; Flush and disable data cache so debugger can see/manipulate data memory\n"
     "   movhi r6, #high(0x4000a000)\n"
     "   add   r6, #low(0x4000a000)\n"
     "; Read cache en bit\n"
     "   ld    r5, [r6]+0x24\n"
     "; Store to p_cache_en\n"
     "   movhi r7, #high(p_cache_en)\n"
     "   add   r7, #low(p_cache_en)\n"
     "   st    r5, [r7]\n"
     "; Flush the cache\n"
     "   mov   r5, #0x1\n"
     "   st    r5, [r6]\n"
     "; Check pending status\n"
     "wait_pend: \n"
     "   ld.cc r5, [r6]+0x28\n"
     "   bne.s wait_pend\n"
     "; Disable the cache\n"
     "   st    r0, [r6]+0x24\n"     
     "; Tell debugger to look at us\n"
     "   movhi r2, #0x1\n"
     "   st    r2, [r3]\n"
     "; Wait for debugger to tell us to go\n"
     "wait: \n"
     "   ldub.cc r4, [r0]+short(_dbg_info)\n"
     "   bne  wait\n"
     "; Lets go!\n"
     "\n"
     "; Reload all registers and return\n"
     "go:\n"
    "; Renable data cache if previously enabled\n"
     "    ld    r5, [r7]\n"
     "    st    r5, [r6]+0x24\n"
     "   icache_flush\n"
     "; renable trace - this could come later but it is useful to\n"
     "; see all the values of all the registers getting reloaded\n"
     "   mov r2, #3\n"
     "   st  r2, [r3]+0x1000\n"
     "   bra go1 ; so trace buffer decode resyncs\n"
     "go1:\n"
     "   ldd r4, [r0]+short(_dbg_info+16*4)\n"
     "   mov psr, r5\n"
     "   mov rtt, r4\n"
     "   ldq r12,[r0]+short(_dbg_info+12*4)\n"
     "   ldq r8, [r0]+short(_dbg_info+8*4)\n"
     "   ldq r4, [r0]+short(_dbg_info+4*4)\n"
     "   ldd r2, [r0]+short(_dbg_info+2*4)\n"
     "   ld  r1, [r0]+short(_dbg_info+1*4)\n"
     "   rti\n");

void __attribute__((noinline)) exit (int status)
{
    while (1) {
	/* Reboot */
        __asm__ ("trap #0");
    }
}

/* _exit and _Exit have exactly the same function as exit */
__asm__ (".global _exit\n"
     ".global _Exit\n"
     ".equiv  _exit, exit\n"
     ".equiv  _Exit, exit");

/* Ensure that a reference to the function __errno is generated here so that
   the symbol gets picked up from libc. Otherwise if the first reference
   to __errno could come from libgloss - in which case end up with link errors */
__asm__ (".equiv  xerrno, __errno");

/********************************************************************************
 * Profiling support
 ********************************************************************************/

#ifdef PROFILING

/* This is the main structure which will be dumped at the end of execution.
   It should be in bss and cleared on startup.
   The arrays are allocated dynmically from the heap via sbrk.
   It is assumed that the heap is initially reset to 0 via the startup code
   above (i.e. it is in .bss) */
struct gmonparam gmonparam;

/* Keep one counter for every 4 bytes of code 
   (power of 2 to avoid divide routine in interupt handler) */ 
static const unsigned scale = 4;

/* Allow for one arc every 32 bytes of code size 
   (power of 2 to avoid divide routine) */
static const unsigned arc_density = 32;

/* We have cpu frequency available in KHZ. We divide it by this number to give
   the number of samples per second */
#define CLOCK_DIVISOR 4
#define SAMPLES_PER_SECOND  (1000/(CLOCK_DIVISOR))

void init_profile (void)
{
    extern char _text[], _etext[];
    struct rawarc* arcs;
    unsigned short* counts;
    unsigned lowpc, highpc, ncounts, maxarcs;
    
    /* Calculate parameters for data structure */
    lowpc  = (unsigned)_text;
    highpc = (unsigned)_etext;
    ncounts = (highpc - lowpc + scale - 1)/scale;
    highpc = lowpc + ncounts * scale;
    maxarcs = (highpc - lowpc)/arc_density;
    
    /* Allocate arrays */
    counts = (unsigned short *) sbrk (ncounts * sizeof(unsigned short));
    arcs = (struct rawarc *) sbrk (maxarcs * sizeof(struct rawarc));

    if (counts == (void*)-1 || arcs == (void*)-1)
        exit(2);
    
    /* Initialize gmonparam data structure */
    gmonparam = (struct gmonparam) { 
        .lowpc = lowpc,
        .highpc = highpc,
        .ncounts = ncounts,
        .maxarcs = maxarcs,
        .narcs = 0,
        .rate = SAMPLES_PER_SECOND,
        .counts = counts,
        .arcs = arcs,
    };
}

void start_profile (void)
{
    /* Set up counter to run profile_handler at the rate SAMPLES_PER_SECOND */
    unsigned reload = _cpu_clock_frequency_khz/CLOCK_DIVISOR; 
    counter1->reload = reload; 
    counter1->value = reload;
    irq[IRQ_COUNTER1].ien = 1;
    counter1->mask = 1;
}

void stop_profile (void)
{
    counter1->mask = 0;
}

/* Update the profile counters for the location we are at when the interrupt
   occurs */
void profile_handler (void) __attribute__((interrupt));
void profile_handler (void)
{
  unsigned pc;
  __asm__ ("mov %0, rtt" : "=r"(pc) : /*no inputs*/);
  
  counter1->expired = 0;

  if (pc < gmonparam.lowpc) return;
  unsigned i = (pc - gmonparam.lowpc)/scale;
  if (i < gmonparam.ncounts)
      gmonparam.counts[i]++;
}

/* Arc counting routine. 
   The compiler inserts calls to mcount. The assembler mcount routine 
   saves the r2-r7 before calling this routine */
void _mcount (unsigned frompc, unsigned selfpc)
{
    struct rawarc *arc = gmonparam.arcs;
    unsigned i;

    /* If arc already exists, increment count */
    for (i = 0; i < gmonparam.narcs; i++) {
        if (arc[i].frompc == frompc && arc[i].selfpc == selfpc) {
            arc[i].count++;
            return;
        }
    }

    /* space for another arc? */
    if (gmonparam.narcs + 1 <= gmonparam.maxarcs)
        arc[gmonparam.narcs++] = (struct rawarc) { 
            .frompc = frompc, 
            .selfpc = selfpc, 
            .count = 1 
        };
}

/* The compiler inserts calls to this routine when we compile with -pg.
   The compiler assumes that only r7 and r15 are clobbered so we have to 
   save r2-r6. The prologue code in _mcount will save what it clobbers in
   r8-r14 */
__asm__(".section .text\n"
    ".balign 4\n"
    ".global mcount\n"
    ".type   mcount, @function\n"
    "mcount:\n"
    "  sub     r1, #7*4\n"
    "  std     r2, [r1]\n"
    "  stq     r4, [r1]+2*4\n"
    "  st      r15,[r1]+6*4\n"
    "  mov     r2, r7\n"
    "  mov     r3, r15\n"
    "  call    _mcount\n"
    "  ld      r15,[r1]+6*4\n"
    "  ldq     r4, [r1]+2*4\n"
    "  ldd     r2, [r1]\n"
    "  add     r1, #7*4\n"
    "  jmp     [r15]\n"
    ".size   mcount, .-mcount\n"); 
#endif
