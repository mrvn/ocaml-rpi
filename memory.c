/* memory.c - Memory management
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

// #include <stdio.h>
#include <stdint.h>
#include "list.h"
#include "printf.h"
#include "string.h"

void delay(uint32_t);

typedef struct Chunk Chunk;
struct Chunk {
    DList all;
    int used;
    union {
	char data[0];
	DList free;
    };
};

enum {
    NUM_SIZES = 32,
    ALIGN = __alignof__(Chunk),
    MIN_SIZE = sizeof(DList),
    HEADER_SIZE = OFFSETOF(Chunk, data),
};

Chunk *free_chunk[NUM_SIZES] = { NULL };
size_t mem_free = 0;
size_t mem_used = 0;
size_t mem_meta = 0;
Chunk *first = NULL;
Chunk *last = NULL;

void memory_chunk_init(Chunk *chunk) {
//    printf("%s(%p)\n", __FUNCTION__, chunk);
    DLIST_INIT(chunk, all);
    chunk->used = 0;
    DLIST_INIT(chunk, free);
}

size_t memory_chunk_size(const Chunk *chunk) {
//    printf("%s(%p)\n", __FUNCTION__, chunk);
    char *end = (char*)(chunk->all.next);
    char *start = (char*)(&chunk->all);
    return (end - start) - HEADER_SIZE;
}

int memory_chunk_slot(size_t size) {
    int n = -1;
    while(size > 0) {
	++n;
	size /= 2;
    }
    return n;
}

void memory_init(void *mem, size_t size) {
    first = (Chunk*)(((intptr_t)mem + ALIGN - 1) & (~(ALIGN - 1)));
    last = ((Chunk*)(((intptr_t)mem + size) & (~(ALIGN - 1)))) - 1;
    Chunk *second = first + 1;
    memory_chunk_init(first);
    memory_chunk_init(second);
    memory_chunk_init(last);
    dlist_insert_after(&first->all, &second->all);
    dlist_insert_after(&second->all, &last->all);
    // mark first/last as used so they never get merged
    first->used = 1;
    last->used = 1;

    size_t len = memory_chunk_size(second);
    int n = memory_chunk_slot(len);
    printf("%s(%p, %#zx) : adding chunk %#zx [%d]\n", __FUNCTION__, mem, size, len, n);
    DLIST_PUSH(&free_chunk[n], second, free);
    mem_free = len - HEADER_SIZE;
    mem_meta = sizeof(Chunk) * 2 + HEADER_SIZE;
}

void *malloc(size_t size) {
    printf("%s(%#zx)\n", __FUNCTION__, size);
    size = (size + ALIGN - 1) & (~(ALIGN - 1));
    if (size < MIN_SIZE) size = MIN_SIZE;
    int n = memory_chunk_slot(size - 1) + 1;
    if (n >= NUM_SIZES) return NULL;
    while(!free_chunk[n]) {
	++n;
	if (n >= NUM_SIZES) return NULL;
    }
    Chunk *chunk = DLIST_POP(&free_chunk[n], free);
    size_t size2 = memory_chunk_size(chunk);
//    printf("@ %p [%#zx]\n", chunk, size2);
    size_t len = 0;
    if (size + sizeof(Chunk) <= size2) {
	Chunk *chunk2 = (Chunk*)((intptr_t)chunk + HEADER_SIZE + size);
	memory_chunk_init(chunk2);
	dlist_insert_after(&chunk->all, &chunk2->all);
	len = memory_chunk_size(chunk2);
	n = memory_chunk_slot(len);
//	printf("  adding chunk @ %p %#zx [%d]\n", chunk2, len, n);
	DLIST_PUSH(&free_chunk[n], chunk2, free);
	mem_meta += HEADER_SIZE;
	mem_free += len - HEADER_SIZE;
    }
    chunk->used = 1;
    mem_free -= size2;
    mem_used += size2 - len - HEADER_SIZE;
    printf("  = %p [%p]\n", chunk->data, chunk);
    return chunk->data;
}

void remove_free(Chunk *chunk) {
    size_t len = memory_chunk_size(chunk);
    int n = memory_chunk_slot(len);
//    printf("%s(%p) : removing chunk %#zx [%d]\n", __FUNCTION__, chunk, len, n);
    DLIST_REMOVE_FROM(&free_chunk[n], chunk, free);
    mem_free -= len - HEADER_SIZE;
}

void push_free(Chunk *chunk) {
    size_t len = memory_chunk_size(chunk);
    int n = memory_chunk_slot(len);
//    printf("%s(%p) : adding chunk %#zx [%d]\n", __FUNCTION__, chunk, len, n);
    DLIST_PUSH(&free_chunk[n], chunk, free);
    mem_free += len - HEADER_SIZE;
}

void free(void *mem) {
    if (mem == NULL) return;
    Chunk *chunk = (Chunk*)((intptr_t)mem - HEADER_SIZE);
    Chunk *next = CONTAINER(Chunk, all, chunk->all.next);
    Chunk *prev = CONTAINER(Chunk, all, chunk->all.prev);
    printf("%s(%p): @%p %#zx [%d]\n", __FUNCTION__, mem, chunk, memory_chunk_size(chunk), memory_chunk_slot(memory_chunk_size(chunk)));
    mem_used -= memory_chunk_size(chunk);
    if (next->used == 0) {
	// merge in next
	remove_free(next);
	dlist_remove(&next->all);
	mem_meta -= HEADER_SIZE;
	mem_free += HEADER_SIZE;
    }
    if (prev->used == 0) {
	// merge to prev
	remove_free(prev);
	dlist_remove(&chunk->all);
	push_free(prev);
	mem_meta -= HEADER_SIZE;
	mem_free += HEADER_SIZE;
    } else {
	// make chunk as free
	chunk->used = 0;
	DLIST_INIT(chunk, free);
	push_free(chunk);
    }
}

void *calloc(size_t nmemb, size_t size) {
    // printf("# %s(%zd, %zd)\n", __FUNCTION__, nmemb, size); // delay(100000000);
    size = nmemb * size;
    void *res = malloc(size);
    memset(res, 0, size);
    return res;
}

void *realloc(void *ptr, size_t size) {
    printf("# %s(%p, %zd)\n", __FUNCTION__, ptr, size);
    delay(100000000);
    Chunk *chunk = (Chunk*)((intptr_t)ptr - HEADER_SIZE);
    size_t old = memory_chunk_size(chunk);
    printf("  old = %zd\n", old);
    if (old >= size) {
	printf("### WARNING: %s(): no shrinking\n", __FUNCTION__);
	return ptr;
    } else {
	void *res = malloc(size);
	memcpy(res, ptr, old);
	free(ptr);
	return res;
    }
}
