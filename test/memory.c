/* memory.c - Memory management test
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
 * Test memory manager.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../memory.c"

#define MEM_SIZE (1024*1024*1024)
char MEM[MEM_SIZE] = { 0 };

#define MAX_BLOCK (1024*1024*2)
#define NUM_SLOTS 1024
void *slot[NUM_SLOTS] = { NULL };
size_t slot_size[NUM_SLOTS] = { 0 };

void check(void) {
    Chunk *t = last;
    DLIST_ITERATOR_BEGIN(first, all, it) {
	assert(CONTAINER(Chunk, all, it->all.prev) == t);
	t = it;
    } DLIST_ITERATOR_END(it);
    for(int i = 0; i < NUM_SIZES; ++i) {
	if (free_chunk[i]) {
	    t = CONTAINER(Chunk, free, free_chunk[i]->free.prev);
	    DLIST_ITERATOR_BEGIN(free_chunk[i], free, it) {
		assert(CONTAINER(Chunk, free, it->free.prev) == t);
		t = it;
	    } DLIST_ITERATOR_END(it);
	}
    }
}

void fill_block(void *mem, size_t size) {
//    printf("%s(%p, %#zx)\n", __FUNCTION__, mem, size);
    void **p = (void **)mem;
    for(size_t i = 0; i < size / sizeof(void*); ++i) {
	*p++ = mem;
    }
}

void check_block(void *mem, size_t size) {
//    printf("%s(%p, %#zx)\n", __FUNCTION__, mem, size);
    void **p = (void **)mem;
    for(size_t i = 0; i < size / sizeof(void*); ++i) {
	if (p[i] != mem) {
	    fprintf(stderr, "ERROR: memory contents changed in block %p [%#zx] @ %p\n", mem, size, &p[i]);
	    assert(0==1);
	}
    }
}

int main() {
    printf("sizeof(DLIST) = %zd\n", sizeof(DList));
    printf("HEADER_SIZE = %d\n", HEADER_SIZE);
    memory_init(MEM, MEM_SIZE);
    printf("mem_free = %#zx, mem_used = %#zx, mem_meta = %#zx\n", mem_free, mem_used, mem_meta);
    for(int i = 0; i < 100000000; ++i) {
	size_t size = random() % MAX_BLOCK;
	int n = random() % NUM_SLOTS;
	check();
	if (slot[n]) {
	    check_block(slot[n], slot_size[n]);
	    free(slot[n]);
	    printf("%d: mem_free = %#zx, mem_used = %#zx, mem_meta = %#zx\n", i, mem_free, mem_used, mem_meta);
	}
	check();
	slot[n] = malloc(size);
	if (slot[n]) {
	    slot_size[n] = size;
	    fill_block(slot[n], slot_size[n]);
	}
	printf("%d: mem_free = %#zx, mem_used = %#zx, mem_meta = %#zx, slot[%d] = %p\n", i, mem_free, mem_used, mem_meta, n, slot[n]);
    }
    for(int i = 0; i < NUM_SLOTS; ++i) {
	if (slot[i]) {
	    check();
	    check_block(slot[i], slot_size[i]);
	    free(slot[i]);
	    printf("mem_free = %#zx, mem_used = %#zx, mem_meta = %#zx\n", mem_free, mem_used, mem_meta);
	}
    }
    check();
    return 0;
}
