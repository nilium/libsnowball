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

#ifndef __ALLOCATOR_WRAPPER_HH__
#define __ALLOCATOR_WRAPPER_HH__


#include <snowball.h>

#include <memory>
#include <limits>


template <typename T>
struct sz_cxx_allocator_t
{

  template <typename U>
  struct rebind
  {
    typedef sz_cxx_allocator_t<U> other;
  };

  typedef T value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;


  sz_allocator_t *const allocator;


  explicit sz_cxx_allocator_t()
  : allocator(sz_default_allocator())
  {
    /* nop */
  }


  explicit sz_cxx_allocator_t(sz_allocator_t *alloc)
  : allocator(alloc ? alloc : sz_default_allocator())
  {
    /* nop */
  }


  ~sz_cxx_allocator_t()
  {
    /* nop */
  }


  sz_cxx_allocator_t(const sz_cxx_allocator_t &other)
  : allocator(other.allocator)
  {
    /* nop */
  }


  template <typename U>
  explicit sz_cxx_allocator_t(const sz_cxx_allocator_t<U> &other)
  : allocator(other.allocator)
  {
    /* nop */
  }


  pointer
  address(reference ref)
  {
    return &ref;
  }


  const_pointer
  address(const_reference ref)
  {
    return &ref;
  }


  pointer
  allocate(size_type count, std::allocator<void>::const_pointer hint = 0)
  {
    (void)hint;
    pointer storage = (pointer)sz_malloc(count * sizeof(value_type), allocator);
    if (!storage) {
      throw std::bad_alloc();
    }
    return storage;
  }


  void
  deallocate(pointer storage, size_type count)
  {
    sz_free(storage, allocator);
  }


  size_type
  max_size() const
  {
    return std::numeric_limits<size_type>::max();
  }


  #if __cplusplus >= 201103L

  template <class U, class... ARGS>
  void
  construct(U *storage, ARGS&&... args)
  {
    ::new((void *)storage) U(std::forward<ARGS>(args)...);
  }

  template <typename U>
  void
  destroy(U *storage)
  {
    ((pointer)storage)->~value_type();
  }

  #else

  void
  construct(pointer storage, const_reference cref)
  {
    ::new((void *)storage) value_type(cref);
  }

  void
  destroy(pointer storage)
  {
    storage->~value_type();
  }

  #endif

};


template <typename T, typename U>
bool operator == (
  const sz_cxx_allocator_t<T> &lhs,
  const sz_cxx_allocator_t<T> &rhs
  )
{
  return lhs.allocator == rhs.allocator;
}


template <typename T, typename U>
bool operator != (
  const sz_cxx_allocator_t<T> &lhs,
  const sz_cxx_allocator_t<T> &rhs
  )
{
  return lhs.allocator != rhs.allocator;
}


#endif /* end __ALLOCATOR_WRAPPER_HH__ include guard */
