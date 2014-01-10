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

#ifndef __BUFSTREAM_HH__
#define __BUFSTREAM_HH__


#include <snowball.h>
#include <string>
#include "allocator_wrapper.hh"


typedef std::basic_string<
#if __cplusplus >= 201103L
  char,
  std::char_traits<char>,
  sz_cxx_allocator_t<char>
#else
  char,
  std::char_traits<char>,
  std::allocator<char>
#endif
  > sz_bufstring_t;


// Returns a sz-stream backed by a stringstream. The underlying buffer
SZ_HIDDEN
sz_stream_t *
sz_buffer_stream(sz_mode_t mode, sz_allocator_t *alloc);

// Returns the underlying string for the stream.
SZ_HIDDEN
sz_bufstring_t
sz_buffer_stream_data(sz_stream_t *stream);


#endif /* end __BUFSTREAM_HH__ include guard */
