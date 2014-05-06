/*
 * Moose Kernel - printf implementation
 * Copyright (C) 2007-2012 Goswin von Brederlow <goswin-v-b@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include "uart.h"
#include "printf.h"

#define BUF_SIZE 4096

static _Bool isdigit(unsigned char c) {
    return ((unsigned char)(c - '0') < 10);
}

ssize_t printf(const char *format, ...) {
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

ssize_t snprintf(char *buf, size_t size, const char *format, ...) {
    va_list args;
    int len;

    va_start(args, format);
    len = vsnprintf(buf, size, format, args);
    va_end(args);

    return len;
}

typedef struct Flags {
    _Bool plus:1;	// Always include a '+' or '-' sign
    _Bool left:1;	// left justified
    _Bool alternate:1;	// 0x prefix
    _Bool space:1;	// space if plus
    _Bool zeropad:1;	// pad with zero
    _Bool sign:1;	// unsigned/signed number
    _Bool upper:1;	// use UPPER case
} Flags;

/* atoi - convert string to int
 * @ptr: pointer to string
 *
 * Returns converted int and leaves ptr pointing to the first character after
 * the number.
 */
int atoi(const char** ptr) {
    const char* s = *ptr;
    int i = 0;
    while(isdigit(*s)) {
	i = i * 10 + (*s++ - '0');
    }
    *ptr = s;
    return i;
}

#define buf_add(c) {		     \
	if (buf < end) { *buf = c; } \
	++buf;			     \
    }

/* print_int - Convert integer to string
 * @buf:       buffer
 * @end:       end of buffer
 * @num:       number to convert
 * @base:      must be 10 or 16
 * @size:      number of bytes to fill
 * @precision: number of digits for floats
 * @flags:     output flags
 *
 * Returns pointer to end of string as it would be for full conversion but
 * only actualy writes up to excluding 'end'.
 */
char* sprint_int(char* buf, char* end, uint64_t num, int base, int width, int precision, Flags flags) {
    const char LOWER[] = "0123456789abcdef";
    const char UPPER[] = "0123456789ABCDEF";
    const char *digits = (flags.upper) ? UPPER : LOWER;
    char tmp[20];

    // Sanity check base
    if (base != 10 && base != 16) return NULL;

    // Check for sign
    _Bool negative = false;
    if (flags.sign) {
	int64_t t = num;
	if (t < 0) {
	    num = -t;
	    negative = true;
	}
    }

    // convert number in reverse order
    int len = 0;
    if (num == 0) { // special case
	tmp[len++] = '0';
    }
    while(num > 0) {
	tmp[len++] = digits[num % base];
	num /= base;
    }
    // Correct presision if number too large
    if (precision < len) precision = len;

    // Account for sign and alternate form
    if (negative || flags.plus) {
	--width;
    }
    if (flags.alternate) {
	width -= 2;
    }
    
    // Put sign if any
    if (negative) {
	buf_add('-');
    } else if (flags.plus) {
	buf_add(flags.space ? ' ' : '+');
    }

    // Put 0x prefix
    if (flags.alternate) {
	buf_add('0');
	buf_add('x');
    }

    // Pad with ' ' if not left aligned
    if (!flags.left) {
	while(precision < width--) buf_add(flags.zeropad ? '0' : ' ');
    }

    // Pad with ' ' or '0' to precision
    while(len < precision--) {
	buf_add(flags.zeropad ? '0' : ' ');
	--width;
    }

    // Put number
    while(len > 0) {
	buf_add(tmp[--len]);
	--width;
    }

    // fill remaining space (flags.left was set)
    while(width-- > 0) buf_add(' ');

    return buf;
}

/* vsnprintf - Format a string and place it in a buffer
 * @buf:    Buffer for result
 * @size:   Size of buffer including trailing '\0'
 * @format: Format string for output
 * @args:   Arguments for format string
 *
 * Returns the number of characters which would be generated for the given
 * input, excluding the trailing '\0', as per ISO C99. If the result is
 * greater than or equal to @size, the rsulting string is truncated.
 */
ssize_t vsnprintf(char* buf, size_t size, const char* format, va_list args) {
    char* start = buf;
    char* end = buf + size;

    while(*format != 0) {
	// Copy normal chars 1:1
	if (*format++ != '%') {
	    buf_add(format[-1]); // format has already advanced
	    continue;
	}

	// Placeholder: %[flags][width][.precision][length]type
	/* Flags:
	 * '+': Always include a '+' or '-' sign for numeric types
	 * '-': Left align output
	 * '#': Alternate form, '0x' prefix for p and x
	 * ' ': Include ' ' for postive numbers
	 * '0': Pad with '0'
	 */
	Flags flags = {false, false, false, false, false, false, false};
    repeat:
	switch(*format++) {
	case '+': flags.plus = true; goto repeat;
	case '-': flags.left = true;  goto repeat;
	case '#': flags.alternate = true; goto repeat;
	case ' ': flags.space = true; goto repeat;
	case '0': flags.zeropad = true; goto repeat;
	default: --format; // undo ++
	}
	/* Width:
	 * '[0-9]'+: use at least this many characters
	 * '*'     : use int from 'args' as width
	 */
	int width = 0;
	if (*format == '*') {
	    ++format;
	    width = va_arg(args, int);
	    if (width < 0) width = 0;
	} else if (isdigit(*format)) {
	    width = atoi(&format);
	}
	/* Precision:
	 * '[0-9]'+: use max this many characters for a string
	 * '*'     : use int from 'args' as precision
	 */
	int precision = -1;
	if (*format == '.') {
	    ++format;
	    if (*format == '*') {
		++format;
		precision = va_arg(args, int);
		if (precision < 0) precision = 0;
	    } else {
		precision = atoi(&format);
	    }
	}
	/* Length:
	 * 'hh': [u]int8_t
	 * 'h' : [u]int16_t
	 * 'l' : [u]int32_t
	 * 'll': [u]int64_t
	 * 'z' : [s]size_t
	 * 't' : ptrdiff_t
	 */
	int length = 4;
	switch(*format++) {
	case 'h':
	    if (*format == 'h') {
		++format; length = 1;
	    } else {
		length = sizeof(short);
	    }
	    break;
	case 'l':
	    if (*format == 'l') {
		++format; length = sizeof(long long);
	    } else {
		length = sizeof(long);
	    }
	    break;
	case 'z':
	    length = sizeof(size_t);
	    break;
	case 't':
	    length = sizeof(intptr_t);
	    break;
	default: --format; // undo ++
	}
	/* Type:
	 * 'd', 'i': signed decimal
	 * 'u'     : unsigned decimal
	 * 'x', 'X': unsigned hexadecimal (UPPER case)
	 * 'p'     : signed hexadecimal of a pointer
	 * 'c'     : character
	 * 's'     : string
	 * '%'     : literal '%'
	 */
	int base = 10;
	uint64_t num = 0;
	switch(*format++) {
	case 'd':
	case 'i':
	    switch(length) {
	    case 1: num = (int8_t) va_arg(args, int); break;
	    case 2: num = (int16_t)va_arg(args, int); break;
	    case 4: num = (int32_t)va_arg(args, int); break;
	    case 8: num = (int64_t)va_arg(args, int64_t); break;
	    }
	    flags.sign = true;
	    if (precision == -1) precision = 0;
	    buf = sprint_int(buf, end, num, base, width, precision, flags);
	    break;
	case 'p':
	    flags.alternate = true;
	    if (precision == -1) precision = 2 * sizeof(void*);
	case 'X': flags.upper = true;
	case 'x': base = 16; flags.space = false; flags.zeropad = true;
	case 'u':
	    switch(length) {
	    case 1: num = (uint8_t) va_arg(args, int); break;
	    case 2: num = (uint16_t)va_arg(args, int); break;
	    case 4: num = (uint32_t)va_arg(args, int); break;
	    case 8: num = (uint64_t)va_arg(args, uint64_t); break;
	    }
	    if (precision == -1) precision = 0;
	    buf = sprint_int(buf, end, num, base, width, precision, flags);
	    break;
	case 'c':
	    buf_add(va_arg(args, int));
	    break;
	case 's': {
	    char* s = va_arg(args, char*);
	    if (precision == -1) {
		while(*s != 0) {
		    buf_add(*s++);
		}
	    } else {
		while(precision > 0 && *s != 0) {
		    --precision;
		    buf_add(*s++);
		}
	    }
	    break;
	}
	case '%':
	    buf_add('%');
	    break;
	default: // Unknown placeholder, rewind and copy '%' verbatim
	    while(*format != '%') --format;
	    buf_add(*format++);
	}
    }
    buf_add(0);
    if (size > 0) end[-1] = 0; // always terminate buffer if there is any
    return (buf - start) - 1;
}
