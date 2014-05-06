#include <stdint.h>
#include "printf.h"
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>

enum {
    // The base address for IRQs
    IRQ_BASE = 0xE000B200,

    // The offsets to reach registers for IRQs
    IRQ_PENDING    = IRQ_BASE + 0x00,
    IRQ_PENDING1   = IRQ_BASE + 0x04,
    IRQ_PENDING2   = IRQ_BASE + 0x08,
    IRQ_FIQCONTROL = IRQ_BASE + 0x0C,
    IRQ_Enable     = IRQ_BASE + 0x18,
    IRQ_Enable1    = IRQ_BASE + 0x10,
    IRQ_Enable2    = IRQ_BASE + 0x14,
    IRQ_Disable    = IRQ_BASE + 0x24,
    IRQ_Disable1   = IRQ_BASE + 0x1C,
    IRQ_Disable2   = IRQ_BASE + 0x20,
};

enum {
    // The base address for Timer.
    TIMER_BASE = 0xE0003000,

    // The offsets to reach registers for the TIMER.
    TIMER_CS     = (TIMER_BASE + 0x00),
    TIMER_CLO    = (TIMER_BASE + 0x04),
    TIMER_CHI    = (TIMER_BASE + 0x08),
    TIMER_C0     = (TIMER_BASE + 0x0C),
    TIMER_C1     = (TIMER_BASE + 0x10),
    TIMER_C2     = (TIMER_BASE + 0x14),
    TIMER_C3     = (TIMER_BASE + 0x18),
};

enum {
    TICKS_PER_SEC = 1000000,
    TICKS_PER_TOCK = 1000000,
};

enum {
    SHIFT_MATCH0, SHIFT_MATCH1, SHIFT_MATCH2, SHIFT_MATCH3
};
enum Flags {
    NONE,
    MATCH0 = 1 << SHIFT_MATCH0,
    MATCH1 = 1 << SHIFT_MATCH1,
    MATCH2 = 1 << SHIFT_MATCH2,
    MATCH3 = 1 << SHIFT_MATCH3,
    _DUMMY = 1 << 31
};

// external init : unit -> unit = "ocaml_thread_init"
CAMLprim value caml_time_init(value unit) {
    CAMLparam1(unit);
    printf("# ocaml_time_init()\n");
    volatile uint32_t *ctrl = (uint32_t*)TIMER_CS;
    volatile uint32_t *lo = (uint32_t*)TIMER_CLO;
    volatile uint32_t *c1 = (uint32_t*)TIMER_C1;
    volatile uint32_t *e1 = (uint32_t*)IRQ_Enable1;
    *c1 = *lo + TICKS_PER_TOCK;
    *ctrl |= MATCH1;
    *e1 |= 2; // Timer 1
    
    CAMLreturn(Val_unit);
}

// external time : unit -> t = "caml_time_time"
CAMLprim value caml_time_time(value unit) {
    CAMLparam1(unit);
    CAMLlocal1(res);
    uint32_t hi = *((uint32_t*)TIMER_CHI);
    uint32_t lo = *((uint32_t*)TIMER_CLO);
    uint64_t t = (((uint64_t)hi) << 32) | lo;
    uint32_t tv_sec = t / TICKS_PER_SEC;
    uint32_t tv_usec = t % TICKS_PER_SEC;
    res = caml_alloc_tuple(2);
    Store_field(res, 0, Val_int(tv_sec));
    Store_field(res, 1, Val_int(tv_usec));
    CAMLreturn(res);
}

extern void caml_record_signal(int signal_number);
/* FIXME: caml_young_limit should be in r10 but sometimes that causes a crash
extern char * caml_code_area_start, * caml_code_area_end, *caml_young_limit, *caml_young_end;
// 32 bits: Represent page table as a 2-level array
#define Pagetable2_log 11
#define Pagetable2_size (1 << Pagetable2_log)
#define Pagetable1_log (Page_log + Pagetable2_log)
#define Pagetable1_size (1 << (32 - Pagetable1_log))
CAMLextern unsigned char * caml_page_table[Pagetable1_size];

#define Pagetable_index1(a) (((uintnat)(a)) >> Pagetable1_log)
#define Pagetable_index2(a) \
  ((((uintnat)(a)) >> Page_log) & (Pagetable2_size - 1))
#define Classify_addr(a) \
  caml_page_table[Pagetable_index1(a)][Pagetable_index2(a)]
#define In_code_area 8
#define Is_in_code_area(pc)		    \
 ( ((char *)(pc) >= caml_code_area_start && \
    (char *)(pc) <= caml_code_area_end)     \
   || (Classify_addr(pc) & In_code_area) )
*/

void time_irq_timer1(uint32_t *regs) {
    volatile uint32_t *ctrl = (uint32_t*)TIMER_CS;
    uint32_t hi   = *(uint32_t*)TIMER_CHI;
    uint32_t lo   = *(uint32_t*)TIMER_CLO;
    volatile uint32_t *c1   = (uint32_t*)TIMER_C1;
    uint32_t e    = *(uint32_t*)IRQ_Enable;
    uint32_t e1   = *(uint32_t*)IRQ_Enable1;
    uint32_t e2   = *(uint32_t*)IRQ_Enable2;
    /*
    printf("### enable  = %#x\n", e);
    printf("### enable1 = %#x\n", e1);
    printf("### enable2 = %#x\n", e2);
    printf("### ctrl = %#x\n", *ctrl);
    printf("### hi   = %#x\n", hi);
    printf("### lo   = %#x\n", lo);
    printf("### c1   = %#x\n", *c1);
    */
    *c1 += TICKS_PER_TOCK;
    *ctrl |= MATCH1;
//    printf("regs[10] = 0x%08x, caml_young_limit = %p, caml_young_end = %p, %s\n", regs[10], caml_young_limit, caml_young_end, Is_in_code_area(regs[15])?"ocaml":"C");
    caml_record_signal(0);
/* FIXME: caml_young_limit should be in r10 but sometimes that causes a crash
    if (Is_in_code_area(regs[15]))
      regs[10] = (uint32_t) caml_young_limit;
*/
}
