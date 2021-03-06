/*
  Copyright (c) 2014 Noel R. Cower

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USEOR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include <snowball.h>


static void *(*sz_malloc_fn)(size_t) = malloc;
static void (*sz_free_fn)(void *) = free;


static
void *
sz_default_malloc(size_t sz, sz_allocator_t *alloc)
{
  (void)alloc;
  return sz_malloc_fn(sz);
}


static
void
sz_default_free(void *ptr, sz_allocator_t *alloc)
{
  (void)alloc;
  sz_free_fn(ptr);
}


static sz_allocator_t default_allocator = {
  sz_default_malloc,
  sz_default_free
};


static sz_allocator_t *default_allocator_ptr = &default_allocator;


SZ_DEF_BEGIN


sz_allocator_t *
sz_default_allocator()
{
  return default_allocator_ptr;
}


void
sz_set_default_allocator(sz_allocator_t *alloc)
{
  default_allocator_ptr = alloc ? alloc : &default_allocator;
}


void
sz_set_default_malloc_free(void *(*malloc_)(size_t), void (*free_)(void *))
{
  sz_malloc_fn = malloc_ ? malloc_ : malloc;
  sz_free_fn = free_ ? free_ : free;
}


SZ_DEF_END
