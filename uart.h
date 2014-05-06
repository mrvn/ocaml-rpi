/* uart.h - serial console driver */
/* Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* Reference material:
 * http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 * Chapter 13: UART
 */

#ifndef MOOSE_KERNEL_UART_H
#define MOOSE_KERNEL_UART_H

#include <stdint.h>

void uart_init(void);
    
/*
 * delay function
 * int32_t delay: number of cycles to delay
 *
 * This just loops <delay> times in a way that the compiler
 * wont optimize away.
 */
void delay(int32_t count);

/*
 * Receive a character via UART0.
 * returns: character received.
 */
char getc(void);

/*
 * Check if data is available via UART0.
 * returns: data available?
 */
_Bool uart_poll(void);

/*
 * print a character to the UART
 * int c: character to print
 */
int putchar(int c);

/*
 * print a string to the UART one character at a time
 * str: 0-terminated string
 */
int puts(const char *str);

/*
 * print error message to the UART and panic
 * msg: error message
 */
void __attribute__((noreturn)) panic(const char *msg);

#endif // #ifndef MOOSE_KERNEL_UART_H
