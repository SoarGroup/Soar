/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2008-
 *     Brian Gerkey gerkey@willowgarage.com
 *                      
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * An implementation of a heap, as seen in "Introduction to Algorithms," by
 * Cormen, Leiserson, and Rivest, pages 140-152.
 */

#ifndef _HEAP_H_
#define _HEAP_H_

#define HEAP_PARENT(i) ((i)/2)
#define HEAP_LEFT(i) (2*(i))
#define HEAP_RIGHT(i) (2*(i)+1)

#ifdef __cplusplus
extern "C" {
#endif

struct heap;

typedef void (*heap_free_elt_fn_t) (void* elt);

typedef struct heap
{
  int len;
  int size;
  heap_free_elt_fn_t free_fn;
  double* A;
  void** data;
} heap_t;

heap_t* heap_alloc(int size, heap_free_elt_fn_t free_fn);
void heap_free(heap_t* h);
void heap_heapify(heap_t* h, int i);
void* heap_extract_max(heap_t* h);
void heap_insert(heap_t* h, double key, void* data);
void heap_dump(heap_t* h);
int heap_valid(heap_t* h);
int heap_empty(heap_t* h);
void heap_reset(heap_t* h);

#ifdef __cplusplus
}
#endif

#endif
