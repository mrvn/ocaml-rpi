#include <stddef.h>
#include <sys/types.h>
#include <signal.h>

#include "uart.h"
#include "printf.h"

#define UNUSED(x) (void)(x)

// error
int errno;
#define EINVAL 22
#define ERANGE 34

// limit
#define LONG_MAX 0x7FFFFFFFL
#define LONG_MIN (-LONG_MAX-1)

// ocaml entry function
extern int caml_startup(char **argv);

// constructors
typedef void (*constructor_t)(void);
extern constructor_t _init_array_start[];
extern constructor_t _init_array_end[];
void kernel_constructors(void) {
    for(constructor_t *fn = _init_array_start; fn != _init_array_end; ++fn) {
        (*fn)();
    }
}

// entry.S
extern uint32_t exception_table[];

double test_double(void) {
    volatile double x = 2.0;
    volatile double y = 3.0;
    double z = x * y;
    asm volatile("vmov %d0, %r0, %r1");
    return z;
}

// enable interrupts
static inline void enable_irq(void) {
    uint32_t t;
    asm volatile("mrs %[t],cpsr; bic %[t], %[t], #0x80; msr cpsr_c, %[t]"
		 : [t]"=r"(t));
}

// disable interrupts
static inline void disable_irq(void) {
    uint32_t t;
    asm volatile("mrs %[t],cpsr; orr %[t], %[t], #0x80; msr cpsr_c, %[t]"
		 : [t]"=r"(t));
}

long int strtol(const char *nptr, char **endptr, int base);
void test_strtol(void) {
    errno = 0; printf("strtol(\"foo\", NULL, 0) = %ld, errno = %d\n", strtol("foo", NULL, 0), errno);
    errno = 0; printf("strtol(\"0\", NULL, -1) = %ld, errno = %d\n", strtol("0", NULL, -1), errno);
    errno = 0; printf("strtol(\"0\", NULL, 1) = %ld, errno = %d\n", strtol("0", NULL, 1), errno);
    errno = 0; printf("strtol(\"0\", NULL, 37) = %ld, errno = %d\n", strtol("0", NULL, 37), errno);
    errno = 0; printf("strtol(\"0\", NULL, 0) = %ld, errno = %d\n", strtol("0", NULL, 0), errno);
    errno = 0; printf("strtol(\"0\", NULL, 8) = %ld, errno = %d\n", strtol("0", NULL, 8), errno);
    errno = 0; printf("strtol(\"0\", NULL, 16) = %ld, errno = %d\n", strtol("0", NULL, 16), errno);
    errno = 0; printf("strtol(\"123\", NULL, 10) = %ld, errno = %d\n", strtol("123", NULL, 10), errno);
    errno = 0; printf("strtol(\"20\", NULL, 16) = %ld, errno = %d\n", strtol("20", NULL, 16), errno);
    errno = 0; printf("strtol(\"78\", NULL, 8) = %ld, errno = %d\n", strtol("78", NULL, 8), errno);
    errno = 0; printf("strtol(\"08\", NULL, 0) = %ld, errno = %d\n", strtol("08", NULL, 0), errno);
    errno = 0; printf("strtol(\"0x20\", NULL, 0) = %ld, errno = %d\n", strtol("0x20", NULL, 0), errno);
    errno = 0; printf("strtol(\"017\", NULL, 0) = %ld, errno = %d\n", strtol("017", NULL, 0), errno);
    errno = 0; printf("strtol(\"0x\", NULL, 0) = %ld, errno = %d\n", strtol("0x", NULL, 0), errno);
    errno = 0; printf("strtol(\"+0x20\", NULL, 0) = %ld, errno = %d\n", strtol("+0x20", NULL, 0), errno);
    errno = 0; printf("strtol(\"-0x20\", NULL, 0) = %ld, errno = %d\n", strtol("-0x20", NULL, 0), errno);
    errno = 0; printf("strtol(\"0x81111111\", NULL, 16) = %ld, errno = %d\n", strtol("0x81111111", NULL, 16), errno);
    errno = 0; printf("strtol(\"-0x81111111\", NULL, 16) = %ld, errno = %d\n", strtol("-0x81111111", NULL, 16), errno);
}

void kernel_main(int zero, int model, void *atags) {
    (void)zero;
    (void)model;
    (void)atags;
    char arg0[] = "ocaml kernel";
    char *argv[] = {arg0, NULL};
    uart_init();
    puts("\n# uart initialized\n");
    // delay(100000000);

    {
	char c;
	printf("# stack = %p\n", &c);
    }
    
    // set exception vector base address register
    asm volatile("mcr p15, 0, %[addr], c12, c0, 0"
		 : : [addr]"r"(exception_table));
    puts("# exception vector set\n");
    // delay(100000000);

    puts("# enabling IRQs\n");
    enable_irq();

    printf("%06d\n", 0);
    
    test_double();
    puts("# doubles tested\n");
    // delay(100000000);

    test_strtol();
    
    printf("model = %d\n", model);
    printf("atags @ %p\n", atags);
    // delay(100000000);
    caml_startup(argv);
    // delay(100000000);
    panic("all done\n");
}

/***************************************************************************
 * math functions                                                          *
 ***************************************************************************/
double acos(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n");
    UNUSED(x);
    return 0;
}
double asin(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n");
    UNUSED(x);
    return 0;
}
double atan(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double atan2(double y, double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    UNUSED(y);
    return 0;
}
double ceil(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double cos(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double cosh(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double exp(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double expm1(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double floor(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double fmod(double x, double y) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    UNUSED(y);
    return 0;
}
double frexp(double x, int *expo) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    UNUSED(expo);
    return 0;
}
double hypot(double x, double y) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    UNUSED(y);
    return 0;
}
double ldexp(double x, int expo) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    UNUSED(expo);
    return 0;
}
double log(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double log10(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double log1p(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double modf(double x, double *iptr) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    UNUSED(iptr);
    return 0;
}
double pow(double x, double y) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    UNUSED(y);
    return 0;
}
double sin(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double sinh(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double sqrt(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double tan(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
double tanh(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}
int __fpclassify(double x) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(x);
    return 0;
}

/***************************************************************************
 * C functions                                                             *
 ***************************************************************************/

// IO
typedef struct FILE {
    int fd;
    char pad[66536];
} FILE;

FILE stdin = { .fd = 0 };
FILE stdout = { .fd = 1 };
FILE stderr = { .fd = 2 };

int fflush(FILE *stream) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(stream);
    return 0;
}

int fputc(int c, FILE *stream) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(stream);
    return putchar(c);
}

int fputs(const char *s, FILE *stream) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(stream);
    return puts(s);
}

ssize_t write(int fd, const void *buf, size_t count) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(fd);
    size_t n = count;
    const char *p = (const char *)buf;
    while(n-- > 0) putchar(*p++);
    return count;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(stream);
    size *= nmemb;
    size_t n = size;
    const char *p = (const char *)ptr;
    while(n-- > 0) putchar(*p++);
    return size;
}

ssize_t read(int fd, void *buf, size_t count) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(fd);
    UNUSED(buf);
    UNUSED(count);
    // FIXME: read from uart
    panic("MISSING: read()");
    return -1;
}

// memory
extern char *_end;
void *end = (void*)(((char*)&_end) + 4);

void *malloc(size_t size) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    size_t *res = (size_t *)end;
    size = (size + 7) & (~7);
    end = (void *)((char *)end + size + sizeof(size_t) * 2);
    *res++ = size;
    printf("# %s(%zd) = %p\n", __FUNCTION__, size, res);
    return res;
}

void free(void *ptr) {
    UNUSED(ptr);
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
}


void *memmove(void *dest, const void *src, size_t n) {
    // puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    char *d = (char *)dest;
    const char *s = (const char *)src;
    if (d < s || (size_t)(d - s) >= n) {
	while(n-- > 0) *d++ = *s++;
    } else {
	d += n;
	s += n;
	while(n-- > 0) *--d = *--s;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while(n-- > 0) *d++ = *s++;
    return dest;
}

void *memset(void *s, int c, size_t n) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    char *p = (char *)s;
    while(n-- > 0) *p++ = c;
    return s;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    const char *p = (const char *)s1;
    const char *q = (const char *)s2;
    int t = 0;
    while(t == 0 && n-- > 0) {
	t = *p++ - *q++;
    }
    return t;
}

void *calloc(size_t nmemb, size_t size) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    size = nmemb * size;
    void *res = malloc(size);
    memset(res, 0, size);
    return res;
}

void *realloc(void *ptr, size_t size) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    size_t *mem = (size_t *)ptr;
    size_t old = mem[-1];
    if (old >= size) {
	return ptr;
    } else {
	void *res = malloc(size);
	memcpy(res, ptr, old);
	free(ptr);
	return res;
    }
}

// exit
void __attribute__((noreturn)) exit(int status) {
    UNUSED(status);
    panic("exit()");
}

// filesystem
int chdir(const char *path) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(path);
    return -1;
}

int close(int fd) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(fd);
    return 0;
}

typedef struct DIR { } DIR;

int closedir(DIR *dirp) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(dirp);
    return 0;
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(fd);
    UNUSED(cmd);
    return -1;
}

char *getcwd(char *buf, size_t size) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(size);
    buf[0] = '/';
    buf[1] = 0;
    return buf;
}

//typedef uint64_t off_t;

off_t lseek64(int fd, off_t offset, int whence) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(fd);
    UNUSED(offset);
    UNUSED(whence);
    return (off_t)-1;
}

// typedef int mode_t;
int open64(const char *pathname, int flags, mode_t mode) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(pathname);
    UNUSED(flags);
    UNUSED(mode);
    return -1;
}

DIR *opendir(const char *name) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(name);
    return NULL;
}

struct dirent;

struct dirent *readdir64(DIR *dirp) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(dirp);
    return NULL;
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(path);
    UNUSED(buf);
    UNUSED(bufsiz);
    return -1;
}

int rename(const char *oldpath, const char *newpath) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(oldpath);
    UNUSED(newpath);
    return -1;
}

int unlink(const char *pathname) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(pathname);
    return -1;
}

struct stat;

int __xstat64(const char *path, struct stat *buf) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(path);
    UNUSED(buf);
    return -1;
}

// processes
char *getenv(const char *name) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(name);
    return NULL;
}

typedef int pid_t;

pid_t getpid(void) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    return 1;
}

pid_t getppid(void) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    return 0;
}

struct rlimit;

int getrlimit(int resource, struct rlimit *rlim) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(resource);
    UNUSED(rlim);
    return -1;
}

struct rusage;

int getrusage(int who, struct rusage *usage) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(who);
    UNUSED(usage);
    return -1;
}

// time
struct timeval;
struct timezone;

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(tv);
    UNUSED(tz);
    return -1;
}

// locale
char *setlocale(int category, const char *locale) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(category);
    UNUSED(locale);
    return NULL;
}

const unsigned short **__ctype_b_loc (void) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    static const unsigned short *ctypes[384];
    return &ctypes[128];
}

// signals
//struct sigaction;

int sigaction(int signum, const struct sigaction *act,
	      struct sigaction *oldact) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(signum);
    UNUSED(act);
    UNUSED(oldact);
    if (oldact) {
	memset(oldact, 0, sizeof(struct sigaction));
	oldact->sa_handler = SIG_DFL;
    }
    return 0;
}

// typedef struct { } sigset_t;

int sigaddset(sigset_t *set, int signum) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(set);
    UNUSED(signum);
    return -1;
}

int sigaltstack (const struct sigaltstack *__restrict ss,
		 struct sigaltstack *__restrict oss) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(ss);
    UNUSED(oss);
    return -1;
}

int sigdelset(sigset_t *set, int signum) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(set);
    UNUSED(signum);
    return -1;
}

int sigemptyset(sigset_t *set) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(set);
    return -1;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(how);
    UNUSED(set);
    UNUSED(oldset);
    return -1;
}

// string
char *strcat(char *dest, const char *src) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    char *p = dest;
    while(*p++);
    while(*src) { *p++ = *src++; }
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    int t = 0;
    while(t != 0 && *s1) {
	t = (*s1++) - (*s2++);
    }
    return t;
}

char *strcpy(char *dest, const char *src) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    char *p = dest;
    while(*src) *p++ = *src++;
    return dest;
}

size_t strlen(const char *s) {
    // puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    size_t res = 0;
    while(*s++) ++res;
    return res;
}

double strtod(const char *nptr, char **endptr) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(nptr);
    UNUSED(endptr);
    return 0.0;
}

int isspace(int c) {
    switch(c) {
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
	return 1;
	break;
    default:
	return 0;
    }
}

int isdigit(int c) {
    if (c >= '0' && c <= '9') return 1;
    return 0;
}

int digit(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 16;
    if (c >= 'a' && c <= 'z') return c - 'a' + 16;
    return 256;
}

long int strtol(const char *nptr, char **endptr, int base) {
    long int res = 0;
    int sign = 1;
    const char *ptr = nptr;
    const char *p = nptr;
//    printf("# %s(%s, %p, %d)\n", __FUNCTION__, nptr, endptr, base);
    // sanity check
    if (!nptr || base == 1 || base < 0 || base > 36) {
	errno = EINVAL;
	goto out;
    }

    // skip leading whitespace
    while(*p != '\0' && isspace(*p)) ++p;
    if (*p == '\0') goto out;

    // parse sign
    if (*p == '+') {
	++p;
    }
    if (*p == '-') {
	++p;
	sign = -1;
    }
    if (*p == '\0') goto out;

    // detect base for hex
    if (base == 0 || base == 16) {
	if (p[0] == '0' && p[1] == 'x' && digit(p[2]) < 16) {
	    ptr = p + 2;
	    base = 16;
	}
    }

    // Handle base detection for octal
    if (base == 0 && p[0] == '0') {
	ptr = p + 1;
	base = 8;
    }

    // Fall back on decimal when unknown
    if (base == 0) {
	base = 10;
    }

    // parse base prefix
    const char *q = p;
    switch(base) {
    case 0:
	if (*q == '0') {
	    ++q;
	    if (*q == 'x' || *q == 'X') {
		++q;
		if (digit(*q) < 16) {
		    base = 16;
		    ptr = q;
		} else {
		    base = 10;
		    ptr = p;
		}
	    } else {
		base = 8;
		ptr = q;
	    }
	} else if (digit(*q) < 10) {
	    base = 10;
	    ptr = p;
	} else {
	    base = 10;
	}
	break;
    case 16:
	if (*q == '0') {
	    ++q;
	    if (*q == 'x' || *q == 'X') {
		++q;
		if (digit(*q) < 16) {
		    ptr = q;
		} else {
		    ptr = p;
		}
	    } else {
		ptr = p;
	    }
	} else if (digit(*q) <= 16) {
	    ptr = p;
	}
	break;
    default:
	if (digit(*q) < base) {
	    ptr = q;
	}
    }
    // parse number
    long int min = LONG_MIN / base;
    int d;
    while((d = digit(*ptr)) < base) {
	if (res < min) {
	    res = LONG_MIN;
	    errno = ERANGE;
	} else {
	    res *= base;
	    if (res < LONG_MIN + d) {
		res = LONG_MIN;
		errno = ERANGE;
	    } else {
		res -= d;
	    }
	}
	++ptr;
    }
    
    out:
//    printf("nptr = '%s', ptr = '%s', res = %ld\n", nptr, ptr, -res);
    if (endptr) *endptr = (char*)ptr;
    if (nptr == ptr) errno = EINVAL;
    if (sign == 1) {
	if (res == LONG_MIN) {
	    errno = ERANGE;
	    return LONG_MAX;
	} else {
	    return -res;
	}
    } else {
	return res;
    }
}

int * __errno_location(void) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    return &errno;
}

char *strerror(int errnum) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(errnum);
    return NULL;
}

// system
int system(const char *command) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(command);
    return -1;
}

// stack protector
void * __stack_chk_guard = NULL;
 
void __stack_chk_guard_setup(void) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    int * p;
    p = (int *) &__stack_chk_guard;
 
    /* If you have the ability to generate random numbers in your kernel then use them,
       otherwise for 32-bit code: */
    *p =  0x00000aff;
}
 
void __attribute__((noreturn)) __stack_chk_fail(void) { 
    panic("__stack_chk_fail()");
}

// printf / scanf

int __sprintf_chk(char * str, int flag, size_t len, const char * format, ...) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); //delay(100000000);
    UNUSED(flag);
    va_list args;

    va_start(args, format);
    len = vsnprintf(str, len, format, args);
    va_end(args);

    return len;
}

#define BUF_SIZE 4096
int sprintf(char * str, const char * format, ...) {
    // puts("# "); puts(__FUNCTION__); puts("()\n"); //delay(100000000);
    va_list args;
    size_t len;

    va_start(args, format);
    len = vsnprintf(str, BUF_SIZE, format, args);
    va_end(args);

    return len;
}

int __fprintf_chk(FILE * stream, int flag, const char * format, ...) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); //delay(100000000);
    UNUSED(stream);
    UNUSED(flag);
    va_list args;
    char buf[BUF_SIZE];
    ssize_t len;
    va_start(args, format);
    len = vsnprintf(buf, BUF_SIZE, format, args);
    va_end(args);
    if (len != -1 && len <= BUF_SIZE) {
	puts(buf);
    } else {
	// FIXME: too long
	panic("printf: too long\n");
    }
    return len;
}

int fprintf(FILE * stream, const char * format, ...) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); //delay(100000000);
    UNUSED(stream);
    va_list args;
    char buf[BUF_SIZE];
    ssize_t len;
    va_start(args, format);
    len = vsnprintf(buf, BUF_SIZE, format, args);
    va_end(args);
    if (len != -1 && len <= BUF_SIZE) {
	puts(buf);
    } else {
	// FIXME: too long
	panic("printf: too long\n");
    }
    return len;
}

int __isoc99_sscanf(const char *format, ...) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(format);
    return -1;
}

// sigsetjmp
typedef struct { } sigjmp_buf;

int __sigsetjmp(sigjmp_buf env, int savesigs) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); // delay(100000000);
    UNUSED(env);
    UNUSED(savesigs);
    return 0;
}

// unwind
int raise (int sig) {
    UNUSED(sig);
    panic("raise()\n");
}

void __aeabi_unwind_cpp_pr0(void) {
    panic("__aeabi_unwind_cpp_pr0()\n");
}

/***************************************************************************
 * dl functions                                                            *
 ***************************************************************************/
void *dlopen(const char *filename, int flag) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(filename);
    UNUSED(flag);
    return 0;
}

int dlclose(void *handle) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(handle);
    return -1;
}

const char *dlerror(void) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    return "ERROR: dlerror()\n";
}

void *dlsym(void *handle, const char *symbol) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    UNUSED(handle); UNUSED(symbol);
    return 0;
}

/***************************************************************************
 * exception handlers                                                      *
 ***************************************************************************/
void dump(uint32_t *regs) {
    static const char *name[] = {"R0 ", "R1 ", "R2 ", "R3 ",
				 "R4 ", "R5 ", "R6 ", "R7 ",
				 "R8 ", "R9 ", "R10", "R11",
				 "R12", "SPu", "LRu", "IPu"};
    printf("regs @ %#8.8x", (uint32_t)regs);
    for(int i = 0; i < 16; ++i) {
	if (i % 4 == 0) putchar('\n');
	printf("%s = %#8.8x    ", name[i], regs[i]);
    }
    putchar('\n');
}

void exception_reset_handler(uint32_t *regs) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    dump(regs);
}

void exception_undefined_handler(uint32_t *regs) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    dump(regs);
}

void exception_syscall_handler(uint32_t *regs) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    dump(regs);
}

void exception_prefetch_abort_handler(uint32_t *regs) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    dump(regs);
}

void exception_data_abort_handler(uint32_t *regs) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    dump(regs);
}

extern void time_irq_timer1(uint32_t *regs);
void exception_irq_handler(uint32_t *regs) {
    UNUSED(regs);
    puts("# "); puts(__FUNCTION__); puts("()\n");
//    dump(regs);
    time_irq_timer1(regs);
}

void exception_fiq_handler(uint32_t *regs) {
    puts("# "); puts(__FUNCTION__); puts("()\n"); delay(100000000);
    dump(regs);
}
