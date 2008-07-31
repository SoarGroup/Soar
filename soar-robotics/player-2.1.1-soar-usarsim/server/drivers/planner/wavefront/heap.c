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
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "heap.h"

heap_t*
heap_alloc(int size, heap_free_elt_fn_t free_fn)
{
  heap_t* h;

  h = calloc(1,sizeof(heap_t));
  assert(h);
  h->size = size;
  h->free_fn = free_fn;
  h->A = calloc(h->size,sizeof(double));
  assert(h->A);
  h->data = calloc(h->size,sizeof(void*));
  assert(h->data);
  h->len = 0;

  return(h);
}

void
heap_free(heap_t* h)
{
  if(h->free_fn)
  {
    while(!heap_empty(h))
      (*h->free_fn)(heap_extract_max(h));
  }
  free(h->data);
  free(h->A);
  free(h);
}

void
heap_heapify(heap_t* h, int i)
{
  int l, r;
  int largest;
  double tmp;
  void* tmp_data;

  l = HEAP_LEFT(i);
  r = HEAP_RIGHT(i);

  if((l < h->len) && (h->A[l] > h->A[i]))
    largest = l;
  else
    largest = i;

  if((r < h->len) && (h->A[r] > h->A[largest]))
    largest = r;

  if(largest != i)
  {
    tmp = h->A[i];
    tmp_data = h->data[i];
    h->A[i] = h->A[largest];
    h->data[i] = h->data[largest];
    h->A[largest] = tmp;
    h->data[largest] = tmp_data;
    heap_heapify(h,largest);
  }
}

int
heap_empty(heap_t* h)
{
  return(h->len == 0);
}

void*
heap_extract_max(heap_t* h)
{
  void* max;

  assert(h->len > 0);

  max = h->data[0];
  h->A[0] = h->A[h->len - 1];
  h->data[0] = h->data[h->len - 1];
  h->len--;
  heap_heapify(h,0);
  return(max);
}

void
heap_insert(heap_t* h, double key, void* data)
{
  int i;

  if(h->len == h->size)
  {
    h->size *= 2;
    h->A = realloc(h->A, h->size * sizeof(double));
    assert(h->A);
    h->data = realloc(h->data, h->size * sizeof(void*));
    assert(h->data);
  }

  h->len++;
  i = h->len - 1;

  while((i > 0) && (h->A[HEAP_PARENT(i)] < key))
  {
    h->A[i] = h->A[HEAP_PARENT(i)];
    h->data[i] = h->data[HEAP_PARENT(i)];
    i = HEAP_PARENT(i);
  }
  h->A[i] = key;
  h->data[i] = data;
}

int
heap_valid(heap_t* h)
{
  int i;
  for(i=1;i<h->len;i++)
  {
    if(h->A[HEAP_PARENT(i)] < h->A[i])
      return(0);
  }
  return(1);
}

void
heap_reset(heap_t* h)
{
  h->len = 0;
}

void
heap_dump(heap_t* h)
{
  int i;
  for(i=0;i<h->len;i++)
    printf("%d: %f\n", i, h->A[i]);
}
