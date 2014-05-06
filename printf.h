/*
 * Moose Kernel - printf interface
 * Copyright (C) 2007-2008 Goswin von Brederlow <goswin-v-b@web.de>
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

#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

//#ifndef ssize_t
//typedef long ssize_t;
//#endif

#define __PRINTFLIKE(__fmt,__varargs) __attribute__((__format__ (__printf__, __fmt, __varargs)))

ssize_t printf(const char *format, ...) __PRINTFLIKE(1, 2);
ssize_t snprintf(char *buf, size_t size, const char *format, ...) __PRINTFLIKE(3, 4);
ssize_t vsnprintf(char *buf, size_t size, const char *format, va_list args);

#endif // #ifndef PRINTF_H
