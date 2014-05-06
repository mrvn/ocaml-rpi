/* list.c - Abstract list handling test case
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
 * Test data structures and helper functions for doubly linked cyclic lists.
 */

#include <stdio.h>
#include <stdint.h>
#include "../list.h"

typedef struct Test {
    int id;
    DList dlist;
} Test;

void test_init(Test *test) {
    static int next_id = 0;
    test->id = next_id++;
    DLIST_INIT(test, dlist);
}

int main() {
    Test *h = NULL;
    Test t1, t2, t3, t4;
    test_init(&t1);
    test_init(&t2);
    test_init(&t3);
    test_init(&t4);
    dlist_insert_after(&t1.dlist, &t2.dlist);
    dlist_insert_after(&t3.dlist, &t4.dlist);
    DLIST_PUSH(&h, &t3, dlist);
    DLIST_PUSH(&h, &t1, dlist);
    DLIST_ITERATOR_BEGIN(h, dlist, loop) {
	printf("@ %p {id = %d, next = %p, prev = %p}\n", loop, loop->id, loop->dlist.next, loop->dlist.prev);
	if (loop->id % 2 == 0) {
	    DLIST_REMOVE_FROM(&h, loop, dlist);
	}
    } DLIST_ITERATOR_END(loop);
    printf("\n");
    DLIST_ITERATOR_BEGIN(h, dlist, loop) {
	printf("@ %p {id = %d, next = %p, prev = %p}\n", loop, loop->id, loop->dlist.next, loop->dlist.prev);
    } DLIST_ITERATOR_END(loop);
    return 0;
}
