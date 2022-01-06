/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.S
 *
 *                 (C) Copyright 2006-2017 Cortus S.A.S
 *                         ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement 
 * from Cortus S.A.S (http://www.cortus.com)
 *
 * $CortusRelease$
 * $FileName$
 *
 * 
 *********************************************************************************/
#pragma GCC optimize ("Os")

#include <stdio.h>
#include <stdlib.h>
#include <machine/cpu.h>
#ifdef __APS__
#include <machine/ic.h>
#else
#include <machine/plic.h>
#include <machine/rvtimer.h>
#endif
#include <machine/uart.h>
#include <machine/counter.h>
#include <machine/dcache.h>
#include <unistd.h>
#include <sys/gmon.h>
#include <machine/trace_buf.h>
#include <machine/sfradr.h>

#ifdef __riscv
#define CSR_MTVEC       0x305
#define CSR_MSCRATCH    0x340
#define CSR_MEPC        0x341
#define CSR_MCAUSE      0x342
#define CSR_DCSR        0x7b0
#endif

/* Debugger printf buffer size.
   Define this to be bigger than 3 in order to enable it in place of uart output
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
#ifdef __riscv
void start(void) __attribute__((section (".init"), used));
void interrupt0_handler          (void) __attribute__((weak));
void interrupt1_handler          (void) __attribute__((weak));
void interrupt2_handler          (void) __attribute__((weak));
void interrupt3_handler          (void) __attribute__((weak));
void interrupt4_handler          (void) __attribute__((weak));
#else
void start(void) __attribute__((section (".init"), used, noreturn, naked));
void critical_error_handler      (void) __attribute__((weak));
void illegal_instruction_handler (void) __attribute__((weak));
void debugger_breakpoint_handler (void);
#endif
void debugger_stop_handler       (void);
#ifdef PROFILING
void profile_handler             (void);
#endif
void interrupt5_handler          (void) __attribute__((weak));
void interrupt6_handler          (void) __attribute__((weak));
void interrupt7_handler          (void) __attribute__((weak));
void interrupt8_handler          (void) __attribute__((weak));
void interrupt9_handler          (void) __attribute__((weak));
void interrupt10_handler         (void) __attribute__((weak));
void interrupt11_handler         (void) __attribute__((weak));
void interrupt12_handler         (void) __attribute__((weak));
void interrupt13_handler         (void) __attribute__((weak));
void interrupt14_handler         (void) __attribute__((weak));
void interrupt15_handler         (void) __attribute__((weak));
void interrupt16_handler         (void) __attribute__((weak));
void interrupt17_handler         (void) __attribute__((weak));
void interrupt18_handler         (void) __attribute__((weak));
void interrupt19_handler         (void) __attribute__((weak));
void interrupt20_handler         (void) __attribute__((weak));
void interrupt21_handler         (void) __attribute__((weak));
void interrupt22_handler         (void) __attribute__((weak));
void interrupt23_handler         (void) __attribute__((weak));
void interrupt24_handler         (void) __attribute__((weak));
void interrupt25_handler         (void) __attribute__((weak));
void interrupt26_handler         (void) __attribute__((weak));
void interrupt27_handler         (void) __attribute__((weak));
void interrupt28_handler         (void) __attribute__((weak));
void interrupt29_handler         (void) __attribute__((weak));
void interrupt30_handler         (void) __attribute__((weak));
void interrupt31_handler         (void) __attribute__((weak));
void interrupt32_handler         (void) __attribute__((weak));
void interrupt33_handler         (void) __attribute__((weak));
void interrupt34_handler         (void) __attribute__((weak));
void interrupt35_handler         (void) __attribute__((weak));
void interrupt36_handler         (void) __attribute__((weak));
void interrupt37_handler         (void) __attribute__((weak));
void interrupt38_handler         (void) __attribute__((weak));
void interrupt39_handler         (void) __attribute__((weak));
void interrupt40_handler         (void) __attribute__((weak));
void interrupt41_handler         (void) __attribute__((weak));
void interrupt42_handler         (void) __attribute__((weak));
void interrupt43_handler         (void) __attribute__((weak));
void interrupt44_handler         (void) __attribute__((weak));
void interrupt45_handler         (void) __attribute__((weak));
void interrupt46_handler         (void) __attribute__((weak));
void interrupt47_handler         (void) __attribute__((weak));
void interrupt48_handler         (void) __attribute__((weak));
void interrupt49_handler         (void) __attribute__((weak));
void interrupt50_handler         (void) __attribute__((weak));
void interrupt51_handler         (void) __attribute__((weak));
void interrupt52_handler         (void) __attribute__((weak));
void interrupt53_handler         (void) __attribute__((weak));
void interrupt54_handler         (void) __attribute__((weak));
void interrupt55_handler         (void) __attribute__((weak));
void interrupt56_handler         (void) __attribute__((weak));
void interrupt57_handler         (void) __attribute__((weak));
void interrupt58_handler         (void) __attribute__((weak));
void interrupt59_handler         (void) __attribute__((weak));
void interrupt60_handler         (void) __attribute__((weak));
void interrupt61_handler         (void) __attribute__((weak));
void interrupt62_handler         (void) __attribute__((weak));
void interrupt63_handler         (void) __attribute__((weak));

#ifdef __riscv
void timer_interrupt_handler     (void) __attribute__((weak));
void software_interrupt_handler  (void) __attribute__((weak));
void ecall_handler               (void) __attribute__((weak));
#endif

#ifdef __riscv
volatile trap_handler_fp trap_vectors[32]
  __attribute__((used)) = 
#else
volatile trap_handler_fp trap_vectors[64]
  __attribute ((section (".vectors"), used)) = 
#endif
{
#ifdef __riscv
    [0]  = interrupt0_handler,
    [1]  = interrupt1_handler,
    [2]  = interrupt2_handler,
    [3]  = interrupt3_handler,
#else
    [0]  = start,
    [1]  = critical_error_handler,
    [2]  = illegal_instruction_handler,
    [3]  = debugger_breakpoint_handler,
#endif
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
    [32] = interrupt32_handler,
    [33] = interrupt33_handler,
    [34] = interrupt34_handler,
    [35] = interrupt35_handler,
    [36] = interrupt36_handler,
    [37] = interrupt37_handler,
    [38] = interrupt38_handler,
    [39] = interrupt39_handler,
    [40] = interrupt40_handler,
    [41] = interrupt41_handler,
    [42] = interrupt42_handler,
    [43] = interrupt43_handler,
    [44] = interrupt44_handler,
    [45] = interrupt45_handler,
    [46] = interrupt46_handler,
    [47] = interrupt47_handler,
    [48] = interrupt48_handler,
    [49] = interrupt49_handler,
    [50] = interrupt50_handler,
    [51] = interrupt51_handler,
    [52] = interrupt52_handler,
    [53] = interrupt53_handler,
    [54] = interrupt54_handler,
    [55] = interrupt55_handler,
    [56] = interrupt56_handler,
    [57] = interrupt57_handler,
    [58] = interrupt58_handler,
    [59] = interrupt59_handler,
    [60] = interrupt60_handler,
    [61] = interrupt61_handler,
    [62] = interrupt62_handler,
    [63] = interrupt63_handler,
};

/********************************************************************************
 * stdio
 * 
 * This is where all the libgloss IO functions end up.
 * These can be overriden in user supplied code to do something more interesting
 * and maybe even look at the FILE* parameter.
 *
 ********************************************************************************/

/* For debugger stub, restore previous cache status */
int p_cache_en = 0;
int pending = 0;

/* Register save area and status info for debugger */
typedef struct dbg_info_t {
    unsigned char status0, status1, status2, status3;
#ifdef __riscv

    unsigned regs[31];
    unsigned mepc, mstatus;
#else
    unsigned r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
    unsigned r12, r13, r14, r15, rtt, psr;
#ifdef __HAVE_MAC__
    unsigned accl, acch;
#endif
#endif
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
    /* Flush, wait for pending done, disable cache */
    p_cache_en = *(unsigned*)(0x4000a024);
    
    *(unsigned*)(0x4000a000) = 0x1;
    do {
      pending = *(unsigned*)(0x4000a028);
    } while (pending);
    
    *(unsigned*)(0x4000a024) = 0;
    
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
    
    /* Enable cache if previously enabled */
    *(unsigned*)(0x4000a024) = p_cache_en;
    
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
#if DATA_CACHE_PRESENT
    dcache->enable = 1;
#endif

    // Initialize the UART to reasonable values
    uart1->config = 0;
    uart1->selclk = 0;
    // Uart speed is 115200 bauds
    uart1->divider = 8*(32000000/115200); /* CPU frequency is 32MHz */
    uart1->enable = 1;
}

/********************************************************************************
 * Execution starts here (linker entry point)
 ********************************************************************************/

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
#ifdef __riscv
    /* Intialize stack pointer and exception register */
    extern void exception_handler(void);
    asm volatile ("la sp, __stack_top");
    asm volatile ("csrw %0,%1": : "i"(CSR_MTVEC), "r"(exception_handler));
    /* Enable ebreak machine mode to enter debug mode */
    asm volatile ("li t6, 0x8000"); 
    asm volatile ("csrw %0,t6": : "i"(CSR_DCSR));
#else
    /* Initialize stack pointer */
    asm ("  movhi   r1, #high(__stack_top)\n"
         "  add     r1, #low(__stack_top)\n");

    /* Initialize pointer to rodata */
    asm ("  movhi   r8, #high(_rodata)\n"
         "  add     r8, #low(_rodata)\n");
#endif

    /* Initialize HW and memory etc and call main */
    while (1) {

        const constructor_t* ctor;
        const unsigned *src;
        unsigned *dst;
        int status;
        
        icache_flush ();
        initialise_hw ();

#ifdef __riscv
        /* Enable jtag interrupt from debugger. 
           This is what happens when you hit the stop button in the
           debugger GUI */
        irq[IRQ_DEBUGGER_STOP].ipl = 0;

        /* Enable interrupts */
        plic->ien[0] = 1 << IRQ_DEBUGGER_STOP;
#else
        /* Enable jtag interrupt from debugger. 
           This is what happens when you hit the stop button in the
           debugger GUI */
        irq[IRQ_DEBUGGER_STOP].ien = 1;
        
        /* Enable all interrupts */
        ic->ien = 1;
#endif
        cpu_int_enable();

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

#ifdef __APS__
/********************************************************************************
 * Monitor stub for debugger
 ********************************************************************************/
asm (".section .text\n"
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
#ifdef __HAVE_MAC__
     "   mov r6, accl\n"
     "   mov r7, acch\n"
     "   stq r4, [r0]+short(_dbg_info+16*4)\n"
#else
     "   std r4, [r0]+short(_dbg_info+16*4)\n"
#endif
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
     "   bne wait_pend\n"
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
#ifdef __HAVE_MAC__
     "   ldq r4, [r0]+short(_dbg_info+16*4)\n"
     "   mov acch, r7\n"
     "   mov accl, r6\n"
#else
     "   ldd r4, [r0]+short(_dbg_info+16*4)\n"
#endif
     "   mov psr, r5\n"
     "   mov rtt, r4\n"
     "   ldq r12,[r0]+short(_dbg_info+12*4)\n"
     "   ldq r8, [r0]+short(_dbg_info+8*4)\n"
     "   ldq r4, [r0]+short(_dbg_info+4*4)\n"
     "   ldd r2, [r0]+short(_dbg_info+2*4)\n"
     "   ld  r1, [r0]+short(_dbg_info+1*4)\n"
     "   rti\n");
#endif

void __attribute__((noinline)) exit (int status)
{
    while (1) {
	/* Reboot */
#ifdef __riscv
        asm ("j start");
#else
        asm ("trap #0");
#endif
    }
}

/* _exit and _Exit have exactly the same function as exit */
asm (".global _exit\n"
     ".global _Exit\n"
     ".equiv  _exit, exit\n"
     ".equiv  _Exit, exit");

/* Ensure that a reference to the function __errno is generated here so that
   the symbol gets picked up from libc. Otherwise if the first reference
   to __errno could come from libgloss - in which case end up with link errors */
asm (".equiv  xerrno, __errno");

#ifdef __riscv
/********************************************************************************
 * Exception Handler
 ********************************************************************************/
asm (".section .text\n"
     ".option nopic\n"
     ".equ plic, 0x40000000\n"
     ".equ ocds, 0x50000000\n"
     ".equ tracebuf, 0x50001000\n"
     ".balign 4\n"
     ".globl exception_handler\n"
     ".type exception_handler, @function\n"
     "exception_handler:\n"
     "# Store non caller save registers to stack\n"
     "  addi   sp, sp, -(16*4)\n"
     "  sw     ra,  (0*4)(sp)\n"
     "  sw     t0,  (1*4)(sp)\n"
     "  sw     t1,  (2*4)(sp)\n"
     "  sw     t2,  (3*4)(sp)\n"
     "  sw     a0,  (4*4)(sp)\n"
     "  sw     a1,  (5*4)(sp)\n"
     "  sw     a2,  (6*4)(sp)\n"
     "  sw     a3,  (7*4)(sp)\n"
     "  sw     a4,  (8*4)(sp)\n"
     "  sw     a5,  (9*4)(sp)\n"
     "  sw     a6, (10*4)(sp)\n"
     "  sw     a7, (11*4)(sp)\n"
     "  sw     t3, (12*4)(sp)\n"
     "  sw     t4, (13*4)(sp)\n"
     "  sw     t5, (14*4)(sp)\n"
     "  sw     t6, (15*4)(sp)\n"
     "  csrr   t0, mcause\n"
     "  li     t1, 0x8000000b\n"
     "  sub    t2, t0, t1\n"
     "  bnez   t2, not_external_interrupt\n"
     
     "# Get address of interrupt handler into t2\n"
     "  li     t2, plic\n"
     "  lw     t2, 4(t2) # t2 <- plic->claim\n"
     "  addi   t1, t2, -4\n"
     "  beqz   t1, debugger_stop_handler\n"
     "  slli   t1, t2, 2\n"
     "  la     t6, trap_vectors\n"
     "  add    t1, t1, t6\n"
     "  lw     t1, 0(t1) # t1 <- trap_vectors[plic->claim]\n"
     "  jalr   t1\n"

     "# Restore caller save registers\n"
     ".globl interrupt_return\n"
     ".type interrupt_return, @function\n"
     "interrupt_return:\n"
     "  lw     ra,  (0*4)(sp)\n"
     "  lw     t0,  (1*4)(sp)\n"
     "  lw     t1,  (2*4)(sp)\n"
     "  lw     t2,  (3*4)(sp)\n"
     "  lw     a0,  (4*4)(sp)\n"
     "  lw     a1,  (5*4)(sp)\n"
     "  lw     a2,  (6*4)(sp)\n"
     "  lw     a3,  (7*4)(sp)\n"
     "  lw     a4,  (8*4)(sp)\n"
     "  lw     a5,  (9*4)(sp)\n"
     "  lw     a6, (10*4)(sp)\n"
     "  lw     a7, (11*4)(sp)\n"
     "  lw     t3, (12*4)(sp)\n"
     "  lw     t4, (13*4)(sp)\n"
     "  lw     t5, (14*4)(sp)\n"
     "  lw     t6, (15*4)(sp)\n"
     "  addi   sp, sp, (16*4)\n"
     "  mret\n"
     "\n"

     "# Handle internal interrupts\n"
     "not_external_interrupt:\n"
     "  la     ra, interrupt_return\n"
     "  addi   t2, t2, 4\n"
     "  bnez   t2, not_timer_interrupt_handler\n"
     "  j      timer_interrupt_handler\n"
     "not_timer_interrupt_handler:\n"
     "  addi   t2, t2, 4\n"
     "  bnez   t2, not_software_interrupt_handler\n"
     "  j      software_interrupt_handler\n"
     "not_software_interrupt_handler:\n"
     "  addi   t2, t0, -3\n"
     "  beqz   t2, enter_debugger\n"
     "  addi   t2, t0, -11\n"
     "  bnez   t2, not_ecall_handler\n"
     "  j      ecall_handler\n"
     "# An exception of some kind which is not breakpoint or ecall\n"
     "not_ecall_handler:\n"
     "  beqz   zero, enter_debugger\n"

     "enter_debugger:\n"
     "# Stop trace\n"
     "  li     t0,  tracebuf\n"
     "  sw     zero, 0(t0)\n"
     "# Debugger is responsible for adjusting mepc if necessary\n"
     "# Save all registers for debugger a few choice CSRs\n"
     "  la     t0,  _dbg_info\n"
     "  lw     t1,   (0*4)(sp)# ra/x1\n"
     "  sw     t1,   (1*4)(t0)\n"
     "  addi   sp, sp, (16*4)\n"
     "  sw     sp,   (2*4)(t0)\n"
     "  addi   sp, sp, -(16*4)\n"
     "  sw     gp,   (3*4)(t0)\n"
     "  sw     tp,   (4*4)(t0)\n"
     "  lw     t1,   (1*4)(sp) # t0/x5\n"
     "  sw     t1,   (5*4)(t0)\n"
     "  lw     t1,   (2*4)(sp) # t1/x6\n"
     "  sw     t1,   (6*4)(t0)\n"
     "  lw     t1,   (3*4)(sp) # t2/x7\n"
     "  sw     t1,   (7*4)(t0)\n"
     "  sw     s0,   (8*4)(t0)\n"
     "  sw     s1,   (9*4)(t0)\n"
     "  sw     a0,  (10*4)(t0)\n"
     "  sw     a1,  (11*4)(t0)\n"
     "  sw     a2,  (12*4)(t0)\n"
     "  sw     a3,  (13*4)(t0)\n"
     "  sw     a4,  (14*4)(t0)\n"
     "  sw     a5,  (15*4)(t0)\n"
     "  sw     a6,  (16*4)(t0)\n"
     "  sw     a7,  (17*4)(t0)\n"
     "  sw     s2,  (18*4)(t0)\n"
     "  sw     s3,  (19*4)(t0)\n"
     "  sw     s4,  (20*4)(t0)\n"
     "  sw     s5,  (21*4)(t0)\n"
     "  sw     s6,  (22*4)(t0)\n"
     "  sw     s7,  (23*4)(t0)\n"
     "  sw     s8,  (24*4)(t0)\n"
     "  sw     s9,  (25*4)(t0)\n"
     "  sw     s10, (26*4)(t0)\n"
     "  sw     s11, (27*4)(t0)\n"
     "  sw     t3,  (28*4)(t0)\n"
     "  sw     t4,  (29*4)(t0)\n"
     "  sw     t5,  (30*4)(t0)\n"
     "  lw     t1,  (15*4)(sp) # t6/x31\n"
     "  sw     t1,  (31*4)(t0)\n"
     "# Save CSRs\n"
     "  csrr   t1,  mepc\n"
     "  sw     t1,  (32*4)(t0)\n"
     "  csrr   t1,  mstatus\n"
     "  sw     t1,  (33*4)(t0)\n"
     "# Reason for monitor entry\n"
     "  csrr   t1,  mcause\n"
     "  bgez   t1,  store_cause\n"
     "  add    t1,  t1, 0x14 # stop_cause_value\n"
     "store_cause:\n"
     "  sb     t1,  (0)(t0)   # status 0\n"
     "# Tell the debugger to look at us\n"
     "wake_debugger:\n"
     "  li     t1, 1\n"
     "  li     t2, ocds # address of ocds register\n"
     "  sb     t1, (2)(t2)\n"
     "# Wait for the debugger to tell us to go\n"
     "wait:\n"
     "  lb     t1, (0)(t0)\n"
     "  bnez   t1, wait\n"
     "# Lets go\n"
     "# Restore CSRs which may have been modified by debugger\n"
     "go:\n"
     "  lw     t1,  (32*4)(t0)\n"
     "  csrw   mepc, t1\n"
     "  lw     t1,  (33*4)(t0)\n"
     "  csrw   mstatus, t1\n"
     "# Renable trace - this could come later but it is useful to\n"
     "# see all the values of all the registers getting reloaded\n"
     "  li     t1,  tracebuf\n"
     "  li     t2,  3\n"
     "  sw     t2,   0(t1)\n"
     "# Restore all registers\n"
     "# place (potentially updated) t0/x5 value into mscratch\n"
     "  lw     ra,   (1*4)(t0)\n"
     "  lw     sp,   (2*4)(t0)\n"
     "  lw     gp,   (3*4)(t0)\n"
     "  lw     tp,   (4*4)(t0)\n"
     "  lw     t1,   (6*4)(t0)\n"
     "  lw     t2,   (7*4)(t0)\n"
     "  lw     s0,   (8*4)(t0)\n"
     "  lw     s1,   (9*4)(t0)\n"
     "  lw     a0,  (10*4)(t0)\n"
     "  lw     a1,  (11*4)(t0)\n"
     "  lw     a2,  (12*4)(t0)\n"
     "  lw     a3,  (13*4)(t0)\n"
     "  lw     a4,  (14*4)(t0)\n"
     "  lw     a5,  (15*4)(t0)\n"
     "  lw     a6,  (16*4)(t0)\n"
     "  lw     a7,  (17*4)(t0)\n"
     "  lw     s2,  (18*4)(t0)\n"
     "  lw     s3,  (19*4)(t0)\n"
     "  lw     s4,  (20*4)(t0)\n"
     "  lw     s5,  (21*4)(t0)\n"
     "  lw     s6,  (22*4)(t0)\n"
     "  lw     s7,  (23*4)(t0)\n"
     "  lw     s8,  (24*4)(t0)\n"
     "  lw     s9,  (25*4)(t0)\n"
     "  lw     s10,  (26*4)(t0)\n"
     "  lw     s11, (27*4)(t0)\n"
     "  lw     t3,  (28*4)(t0)\n"
     "  lw     t4,  (29*4)(t0)\n"
     "  lw     t5,  (30*4)(t0)\n"
     "  lw     t6,  (31*4)(t0)\n"
     "  lw     t0,   (5*4)(t0) # t0/x5\n"
     "  mret\n"
    );

asm (".section .text\n"
     ".equ plic, 0x40000000\n"
     ".equ ocds, 0x50000000\n"
     ".globl debugger_stop_handler\n"
     ".type debugger_stop_handler, @function\n"
     "debugger_stop_handler:\n"
     "# Write mcause for debugger\n"
     "  li     t1, 0x14\n"
     "  csrw   mcause, t1\n"
     "# Clear stop request in brkpts module\n"
     "  li     t1, ocds # address of ocds register\n"
     "  sb     zero, (1)(t1)\n"
     "# Clear pending request in plic\n"
     "  li     t1, plic\n"
     "  sw     t2, 4(t1) #t2 contains claimed vector\n"
     "# Go to monitor\n"
     "  j      enter_debugger\n"
     
    );

void timer_interrupt_handler (void)
{
    /* Clear interrupt */
    rvtimer->compare_h = 0xffffffff;
    rvtimer->compare_l = 0xffffffff;
}

void software_interrupt_handler (void)
{

}

void ecall_handler (void)
{

}
#endif

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
#ifdef __riscv
    irq[IRQ_COUNTER1].ipl = 0;

    plic->ien[0] = plic->ien[0] | (1 << IRQ_COUNTER1);
#else
    irq[IRQ_COUNTER1].ien = 1;
#endif
    counter1->mask = 1;
}

void stop_profile (void)
{
    counter1->mask = 0;
}

/* Update the profile counters for the location we are at when the interrupt
   occurs */
#ifdef __APS__
void profile_handler (void) __attribute__((interrupt));
#endif
void profile_handler (void)
{
  unsigned pc;
#ifdef __riscv
  asm ("csrr %0, mepc" : "=r"(pc) : /*no inputs*/);
#else
  asm ("mov %0, rtt" : "=r"(pc) : /*no inputs*/);
#endif
  
  counter1->expired = 0;

  if (pc < gmonparam.lowpc) return;
  unsigned i = (pc - gmonparam.lowpc)/scale;
  if (i < gmonparam.ncounts)
      gmonparam.counts[i]++;

#ifdef __riscv
  /* Completion message */
  plic->claim = IRQ_COUNTER1;
#endif
  
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

#ifdef __riscv
/* The compiler inserts calls to this routine when we compile with -pg.
 */
asm(".section .text\n"
    ".balign 4\n"
    ".global mcount\n"
    ".type   mcount, @function\n"
    "mcount:\n"
    "  addi     sp, sp, -16*4\n"
    "  sw     ra,  (0*4)(sp)\n"
    "  sw     t0,  (1*4)(sp)\n"
    "  sw     t1,  (2*4)(sp)\n"
    "  sw     t2,  (3*4)(sp)\n"
    "  sw     a0,  (4*4)(sp)\n"
    "  sw     a1,  (5*4)(sp)\n"
    "  sw     a2,  (6*4)(sp)\n"
    "  sw     a3,  (7*4)(sp)\n"
    "  sw     a4,  (8*4)(sp)\n"
    "  sw     a5,  (9*4)(sp)\n"
    "  sw     a6, (10*4)(sp)\n"
    "  sw     a7, (11*4)(sp)\n"
    "  sw     t3, (12*4)(sp)\n"
    "  sw     t4, (13*4)(sp)\n"
    "  sw     t5, (14*4)(sp)\n"
    "  sw     t6, (15*4)(sp)\n"
    "  mv     a1, ra\n"
    "  call   _mcount\n"
    "  lw     ra,  (0*4)(sp)\n"
    "  lw     t0,  (1*4)(sp)\n"
    "  lw     t1,  (2*4)(sp)\n"
    "  lw     t2,  (3*4)(sp)\n"
    "  lw     a0,  (4*4)(sp)\n"
    "  lw     a1,  (5*4)(sp)\n"
    "  lw     a2,  (6*4)(sp)\n"
    "  lw     a3,  (7*4)(sp)\n"
    "  lw     a4,  (8*4)(sp)\n"
    "  lw     a5,  (9*4)(sp)\n"
    "  lw     a6, (10*4)(sp)\n"
    "  lw     a7, (11*4)(sp)\n"
    "  lw     t3, (12*4)(sp)\n"
    "  lw     t4, (13*4)(sp)\n"
    "  lw     t5, (14*4)(sp)\n"
    "  lw     t6, (15*4)(sp)\n"
    "addi     sp, sp, 16*4\n"
    "ret\n"
    ".size   mcount, .-mcount\n"); 
#else
/* The compiler inserts calls to this routine when we compile with -pg.
   The compiler assumes that only r7 and r15 are clobbered so we have to 
   save r2-r6. The prologue code in _mcount will save what it clobbers in
   r8-r14 */
asm(".section .text\n"
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
#endif
