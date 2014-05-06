/* list.h - Abstract list handling
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
 * Data structures and helper functions for doubly linked cyclic lists.
 */

#ifndef OCAML_RPI__LIST_H
#define OCAML_RPI__LIST_H

typedef struct DList DList;
struct DList {
    DList *next;
    DList *prev;
};

// initialize a one element DList
static inline void dlist_init(DList *dlist) {
//    printf("%s(%p)\n", __FUNCTION__, dlist);
    dlist->next = dlist;
    dlist->prev = dlist;
}

// insert d2 after d1
static inline void dlist_insert_after(DList *d1, DList *d2) {
//    printf("%s(%p, %p)\n", __FUNCTION__, d1, d2);
    DList *n1 = d1->next;
    DList *e2 = d2->prev;

    d1->next = d2;
    d2->prev = d1;
    e2->next = n1;
    n1->prev = e2;
}

// insert d2 before d1
static inline void dlist_insert_before(DList *d1, DList *d2) {
//    printf("%s(%p, %p)\n", __FUNCTION__, d1, d2);
    DList *e1 = d1->prev;
    DList *e2 = d2->prev;

    e1->next = d2;
    d2->prev = e1;
    e2->next = d1;
    d1->prev = e2;
}

// remove d from the list
static inline void dlist_remove(DList *d) {
//    printf("%s(%p)\n", __FUNCTION__, d);
    d->prev->next = d->next;
    d->next->prev = d->prev;
    d->next = d;
    d->prev = d;    
}

// push d2 to the front of the d1p list
static inline void dlist_push(DList **d1p, DList *d2) {
//    printf("%s(%p, %p)\n", __FUNCTION__, d1p, d2);
    if (*d1p != NULL) {
	dlist_insert_before(*d1p, d2);
    }
    *d1p = d2;
}

// pop the front of the dp list
static inline DList * dlist_pop(DList **dp) {
//    printf("%s(%p)\n", __FUNCTION__, dp);
    DList *d1 = *dp;
    DList *d2 = d1->next;
    dlist_remove(d1);
    if (d1 == d2) {
	*dp = NULL;
    } else {
	*dp = d2;
    }
    return d1;
}

// remove d2 from the list, advancing d1p if needed
static inline void dlist_remove_from(DList **d1p, DList *d2) {
//    printf("%s(%p, %p)\n", __FUNCTION__, d1p, d2);
    if (*d1p == d2) {
	dlist_pop(d1p);
    } else {
	dlist_remove(d2);
    }
}

#define CONTAINER(C, l, v) ((C*)(((char*)v) - (intptr_t)&(((C*)0)->l)))
#define OFFSETOF(TYPE, MEMBER)  __builtin_offsetof (TYPE, MEMBER)

#define DLIST_INIT(v, l) dlist_init(&v->l)

#define DLIST_REMOVE_FROM(h, d, l)					\
    {									\
	typeof(**h) **h_ = h, *d_ = d;					\
	DList *head = &(*h_)->l;					\
	dlist_remove_from(&head, &d_->l);					\
	if (head == NULL) {						\
	    *h_ = NULL;							\
	} else {							\
	    *h_ = CONTAINER(typeof(**h), l, head);			\
	}								\
    }

#define DLIST_PUSH(h, v, l)						\
    {									\
	typeof(*v) **h_ = h, *v_ = v;					\
	DList *head = &(*h_)->l;					\
	if (*h_ == NULL) head = NULL;					\
	dlist_push(&head, &v_->l);					\
	*h_ = CONTAINER(typeof(*v), l, head);				\
    }

#define DLIST_POP(h, l)							\
    ({									\
	typeof(**h) **h_ = h;						\
	DList *head = &(*h_)->l;					\
	DList *res = dlist_pop(&head);					\
	if (head == NULL) {						\
	    *h_ = NULL;							\
	} else {							\
	    *h_ = CONTAINER(typeof(**h), l, head);			\
	}								\
	CONTAINER(typeof(**h), l, res);					\
    })

#define DLIST_ITERATOR_BEGIN(h, l, it)					\
    {									\
        typeof(*h) *h_ = h;						\
	DList *last_##it = h_->l.prev, *iter_##it = &h_->l, *next_##it;	\
	do {								\
	    if (iter_##it == last_##it) {				\
		next_##it = NULL;					\
	    } else {							\
		next_##it = iter_##it->next;				\
	    }								\
	    typeof(*h)* it = CONTAINER(typeof(*h), l, iter_##it);

#define DLIST_ITERATOR_END(it)						\
	} while((iter_##it = next_##it));				\
    }

#endif // #ifndef OCAML_RPI__LIST_H
