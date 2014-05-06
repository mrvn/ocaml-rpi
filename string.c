/* string.c - string and memory functions
 * Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --
 *
 * string and memory functions
 */

#include "string.h"
#include "printf.h"

void delay(uint32_t);

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
    printf("# %s(dest=%p, src=%p, size=%zd)\n", __FUNCTION__, dest, src, n);
    delay(100000000);
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while(n-- > 0) *d++ = *s++;
    return dest;
}

void *memset(void *s, int c, size_t n) {
    printf("# %s(s = %p, c = %#2x, n = %zd)\n", __FUNCTION__, s, c, n);
    // delay(100000000);
    char *p = (char *)s;
    while(n-- > 0) *p++ = c;
    return s;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    printf("# %s(s1=%p, s2=%p, size=%zd)\n", __FUNCTION__, s1, s2, n);
    delay(100000000);
    const char *p = (const char *)s1;
    const char *q = (const char *)s2;
    int t = 0;
    while(t == 0 && n-- > 0) {
	t = *p++ - *q++;
    }
    return t;
}

char *strcat(char *dest, const char *src) {
    printf("# %s(dest=%p, src=%p)\n", __FUNCTION__, dest, src);
    delay(100000000);
    char *p = dest;
    while(*p++);
    while(*src) { *p++ = *src++; }
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    printf("# %s(s1=%p, s2=%p)\n", __FUNCTION__, s1, s2);
    delay(100000000);
    int t = 0;
    while(t != 0 && *s1) {
	t = (*s1++) - (*s2++);
    }
    return t;
}

char *strcpy(char *dest, const char *src) {
    printf("# %s(dest=%p, src=%p)\n", __FUNCTION__, dest, src);
    // delay(100000000);
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
