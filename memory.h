/* memory.h - Memory management
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
 * Manage chunks of memory implementing malloc and free.
 */

#ifndef OCAML_RPI__MEMORY_H
#define OCAML_RPI__MEMORY_H

#include <stdint.h>
#include "list.h"

extern size_t mem_free;
extern size_t mem_used;
extern size_t mem_meta;

void memory_init(void *mem, size_t size);
void *malloc(size_t size);
void free(void *mem);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

#endif // #ifndef OCAML_RPI__MEMORY_H
