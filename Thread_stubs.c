#include "printf.h"
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/callback.h>

#define THREAD_STACK_SIZE 1024*1024
#define UNUSED(x) (void)(x)

/* The infos on threads (allocated via malloc()) */

struct caml_thread_struct {
  struct caml_thread_struct * next;  /* Double linking of running threads */
  struct caml_thread_struct * prev;
  char * top_of_stack;          /* Top of stack for this thread (approx.) */
  char * bottom_of_stack;       /* Saved value of caml_bottom_of_stack */
  uintnat last_retaddr;         /* Saved value of caml_last_return_address */
  value * gc_regs;              /* Saved value of caml_gc_regs */
  char * exception_pointer;     /* Saved value of caml_exception_pointer */
  struct caml__roots_block * local_roots; /* Saved value of local_roots */
  int backtrace_pos;            /* Saved backtrace_pos */
  code_t * backtrace_buffer;    /* Saved backtrace_buffer */
  value backtrace_last_exn;     /* Saved backtrace_last_exn (root) */

    void *stack;
};

typedef struct caml_thread_struct * caml_thread_t;

/* The descriptor for the currently executing thread */
static caml_thread_t curr_thread = NULL;

/* Hook for scanning the stacks of the other threads */

typedef void (*scanning_action) (value, value *);
static void (*prev_scan_roots_hook) (scanning_action);
extern void (*scan_roots_hook) (scanning_action);
extern void (*enter_blocking_section_hook) (void);
extern void (*leave_blocking_section_hook) (void);

extern char *caml_bottom_of_stack;
extern uintnat caml_last_return_address;
extern value * caml_gc_regs;
extern char * caml_exception_pointer;
extern int caml_backtrace_pos;
extern code_t * caml_backtrace_buffer;
extern value caml_backtrace_last_exn;

#define CRASH asm volatile("mov r11, #0xfffffff0; bx r11")

/*
void switch_stack(void ** old_stack_p, void * new_stack) {
    register void ** r0 asm("r0") = old_stack_p;
    register void *  r1 asm("r1") = new_stack;
    asm volatile("push {r2-r12,r14}");
    asm volatile("str  sp, [%[old_stack_p], #0]"
		 : : [old_stack_p]"r"(r0));
    asm volatile("mov  sp, %[new_stack]"
		 : : [new_stack]"r"(r1));
    asm volatile("pop  {r2-r12,r14}");
    
}
*/
extern void switch_thread(void ** old_stack_p, void * new_stack);
extern void starter_stub(caml_thread_t thread, value fn);

void schedule(void) {
    printf("# schedule()\n");
    if (curr_thread && (curr_thread != curr_thread->next)) {
	/* Save the stack-related global variables in the thread descriptor
	   of the current thread */
	curr_thread->bottom_of_stack = caml_bottom_of_stack;
	curr_thread->last_retaddr = caml_last_return_address;
	curr_thread->gc_regs = caml_gc_regs;
	curr_thread->exception_pointer = caml_exception_pointer;
	curr_thread->local_roots = local_roots;
	curr_thread->backtrace_pos = backtrace_pos;
	curr_thread->backtrace_buffer = backtrace_buffer;
	curr_thread->backtrace_last_exn = backtrace_last_exn;

	// switch threads
	printf("# switching: old_stack = %p, new_stack = %p\n", curr_thread->stack, curr_thread->next->stack);
	switch_thread(&curr_thread->stack, curr_thread->next->stack);
	printf("# switched: old_stack = %p, new_stack = %p\n", curr_thread->stack, curr_thread->next->stack);
	curr_thread = curr_thread->next;

	/* Load the stack-related global variables in the thread descriptor
	   of the current thread */
	caml_bottom_of_stack = curr_thread->bottom_of_stack;
	caml_last_return_address = curr_thread->last_retaddr;
	caml_gc_regs = curr_thread->gc_regs;
	caml_exception_pointer = curr_thread->exception_pointer;
	local_roots = curr_thread->local_roots;
	backtrace_pos = curr_thread->backtrace_pos;
	backtrace_buffer = curr_thread->backtrace_buffer;
	backtrace_last_exn = curr_thread->backtrace_last_exn;
    }
}

CAMLextern void caml_do_local_roots(scanning_action f, char * bottom_of_stack,
                                    uintnat last_retaddr, value * gc_regs,
                                    struct caml__roots_block * local_roots);

static void caml_thread_scan_roots(scanning_action action)
{
//    printf("# ocaml_thread_scan_roots()\n");
    caml_thread_t th;

    th = curr_thread;
    do {
//	printf("#   thread @ %p\n", th);
//	(*action)(th->descr, &th->descr);
	(*action)(th->backtrace_last_exn, &th->backtrace_last_exn);
	/* Don't rescan the stack of the current thread, it was done already */
	if (th != curr_thread) {
	    if (th->bottom_of_stack != NULL) {
//		printf("#     with local roots\n");
//		printf("# bottom_of_stack = %p, last_retaddr = %x, gc_regs = %p, local_roots = %p\n",
//		       th->bottom_of_stack, (uint32_t)th->last_retaddr,
//		       th->gc_regs, th->local_roots);
		do_local_roots(action, th->bottom_of_stack, th->last_retaddr,
			       th->gc_regs, th->local_roots);
	    }
	}
	th = th->next;
    } while (th != curr_thread);
    /* Hook */
    if (prev_scan_roots_hook != NULL) (*prev_scan_roots_hook)(action);
}

/* Hooks for enter_blocking_section and leave_blocking_section */

static void caml_thread_enter_blocking_section(void)
{
    printf("# caml_thread_enter_blocking_section()\n");
}

static void caml_thread_leave_blocking_section(void)
{
    printf("# caml_thread_leave_blocking_section()\n");
//    schedule();
}


/* Hooks for I/O locking */
typedef off_t file_offset;
#ifndef IO_BUFFER_SIZE
#define IO_BUFFER_SIZE 4096
#endif
struct channel {
  int fd;                       /* Unix file descriptor */
  file_offset offset;           /* Absolute position of fd in the file */
  char * end;                   /* Physical end of the buffer */
  char * curr;                  /* Current position in the buffer */
  char * max;                   /* Logical end of the buffer (for input) */
  void * mutex;                 /* Placeholder for mutex (for systhreads) */
  struct channel * next, * prev;/* Double chaining of channels (flush_all) */
  int revealed;                 /* For Cash only */
  int old_revealed;             /* For Cash only */
  int refcount;                 /* For flush_all and for Cash */
  int flags;                    /* Bitfield */
  char buff[IO_BUFFER_SIZE];    /* The buffer itself */
};
extern void (*caml_channel_mutex_free)(struct channel *);
extern void (*caml_channel_mutex_lock)(struct channel *);
extern void (*caml_channel_mutex_unlock)(struct channel *);
extern void (*caml_channel_mutex_unlock_exn)(void);
struct channel * last_channel_locked = NULL;

#define DEBUG_IO_MUTEX 0

static void caml_io_mutex_free(struct channel *chan) {
    if (DEBUG_IO_MUTEX) printf("# caml_io_mutex_free(%p)\n", chan);
    // Nothing to do
    UNUSED(chan);
}

static int MUTEX_LOCKED = 0;
static void caml_io_mutex_lock(struct channel *chan) {
    if (DEBUG_IO_MUTEX) printf("# caml_io_mutex_lock(%p)\n", chan);
    while(1) {
	if (chan->mutex == NULL) {
	    chan->mutex = &MUTEX_LOCKED;
	    last_channel_locked = chan;
	    if (DEBUG_IO_MUTEX) printf("# caml_io_mutex_lock(%p): locked\n", chan);
	    return;
	}
	schedule();
    }
}

static void caml_io_mutex_unlock(struct channel *chan) {
    if (DEBUG_IO_MUTEX) printf("# caml_io_mutex_unlock(%p) [%p]\n", chan, chan->mutex);
    chan->mutex = NULL;
    last_channel_locked = NULL;
}

static void caml_io_mutex_unlock_exn(void) {
    if (DEBUG_IO_MUTEX) printf("# caml_io_mutex_unlock_exn(): last = %p\n", last_channel_locked);
    if (last_channel_locked != NULL) caml_io_mutex_unlock(last_channel_locked);
}


void starter(caml_thread_t th, value fn) {
    printf("# starter()\n");
    curr_thread = th;

    /* Load the stack-related global variables in the thread descriptor
       of the current thread */
    caml_bottom_of_stack = curr_thread->bottom_of_stack;
    caml_last_return_address = curr_thread->last_retaddr;
    caml_gc_regs = curr_thread->gc_regs;
    caml_exception_pointer = curr_thread->exception_pointer;
    local_roots = curr_thread->local_roots;
    backtrace_pos = curr_thread->backtrace_pos;
    backtrace_buffer = curr_thread->backtrace_buffer;
    backtrace_last_exn = curr_thread->backtrace_last_exn;

    // callback closure
    callback_exn(fn, Val_unit);
    CRASH;
}

// FIXME: throw exception on failure
CAMLprim value caml_thread_create(value fn)
{
    CAMLparam1(fn);
    caml_thread_t th;

    th = (caml_thread_t) malloc(sizeof(struct caml_thread_struct));
    if (th != NULL) {
	uint32_t *stack = stat_alloc(THREAD_STACK_SIZE);
	if (stack == NULL) {
	    free(th);
	    th = NULL;
	} else {
	    uint32_t *top = stack + THREAD_STACK_SIZE / sizeof(uint32_t);
	    th->bottom_of_stack = NULL;
	    th->top_of_stack = (char *)top;
	    th->last_retaddr = 1;
	    th->gc_regs = NULL;
	    th->exception_pointer = NULL;
	    th->local_roots = NULL;
	    th->backtrace_pos = 0;
	    th->backtrace_buffer = NULL;
	    th->backtrace_last_exn = Val_unit;
	    
	    // Build stack frame foro starter_stub
	    *--top = (uint32_t)starter; // LR
	    *--top = 3; // r3
	    *--top = 2; // r2
	    *--top = (uint32_t)fn; // r1
	    *--top = (uint32_t)th; // r0
	    // Build stack frame for schedule
	    *--top = 0; // d15
	    *--top = 0; // d15
	    *--top = 0; // d14
	    *--top = 0; // d14
	    *--top = 0; // d13
	    *--top = 0; // d13
	    *--top = 0; // d12
	    *--top = 0; // d12
	    *--top = 0; // d11
	    *--top = 0; // d11
	    *--top = 0; // d10
	    *--top = 0; // d10
	    *--top = 0; // d9
	    *--top = 0; // d9
	    *--top = 0; // d8
	    *--top = 0; // d8
	    *--top = (uint32_t)starter_stub; // LR
	    *--top = 12; // r12 scratch
	    *--top = 11; // r11
	    *--top = 10; // r10
	    *--top = 9; // r9
	    *--top = 8; // r8
	    *--top = 7; // r7
	    *--top = 6; // r6
	    *--top = 5; // r5
	    *--top = 4; // r4

	    th->stack = top;
	    
	    /* Add thread info block to the list of threads */
	    th->next = curr_thread->next;
	    th->prev = curr_thread;
	    curr_thread->next->prev = th;
	    curr_thread->next = th;

	    // start thread before the GC can clean up the closure
	    schedule();
	}
    }
    
    CAMLreturn((value)th);
}

// external init : unit -> unit = "ocaml_thread_init"
CAMLprim value ocaml_thread_init(value unit) {
    CAMLparam1(unit);
    char c;
    printf("# ocaml_thread_init()\n");
    /* Protect against repeated initialization (PR#1325) */
    if (curr_thread != NULL) return Val_unit;

    /* Set up a thread info block for the current thread */
    curr_thread =
	(caml_thread_t) stat_alloc(sizeof(struct caml_thread_struct));
    curr_thread->bottom_of_stack = NULL;
    curr_thread->top_of_stack = &c;
    curr_thread->last_retaddr = 1;
    curr_thread->gc_regs = NULL;
    curr_thread->exception_pointer = NULL;
    curr_thread->local_roots = NULL;
    curr_thread->backtrace_pos = 0;
    curr_thread->backtrace_buffer = NULL;
    curr_thread->backtrace_last_exn = Val_unit;

    curr_thread->next = curr_thread;
    curr_thread->prev = curr_thread;
    /* The stack-related fields will be filled in at the next
       enter_blocking_section */

    /* Set up the hooks */
    prev_scan_roots_hook = scan_roots_hook;
    scan_roots_hook = caml_thread_scan_roots;
/*
    enter_blocking_section_hook = caml_thread_enter_blocking_section;
    leave_blocking_section_hook = caml_thread_leave_blocking_section;
*/
    caml_channel_mutex_free = caml_io_mutex_free;
    caml_channel_mutex_lock = caml_io_mutex_lock;
    caml_channel_mutex_unlock = caml_io_mutex_unlock;
    caml_channel_mutex_unlock_exn = caml_io_mutex_unlock_exn;
    // caml_termination_hook = st_thread_exit;

    CAMLreturn(Val_unit);
}

// external signal : int -> unit = "ocaml_thread_signal"
extern void caml_record_signal(int signal_number);
CAMLprim value caml_thread_signal(value signal_number) {
    CAMLparam1(signal_number);
    printf("# ocaml_thread_signal(%d)\n", Int_val(signal_number));
    caml_record_signal(Int_val(signal_number));
    CAMLreturn(Val_unit);
}

