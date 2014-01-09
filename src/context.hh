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

#ifndef __SZ_SNOWBALL__CONTEXT_HH__
#define __SZ_SNOWBALL__CONTEXT_HH__


#include <snowball.h>


struct SZ_HIDDEN s_sz_context
{
  mutable const char *  error;
  sz_allocator_t *      ctx_alloc;

  sz_stream_t *         stream;
  off_t                 stream_pos;


  s_sz_context(sz_allocator_t *alloc);

  virtual
  ~s_sz_context() = 0;

  virtual
  sz_mode_t
  mode() const = 0;

  sz_response_t
  file_error() const;

  virtual
  sz_response_t
  open() = 0;

  virtual
  sz_response_t
  flush() = 0;

  virtual
  sz_response_t
  close() = 0;
};


// Check whether ctx is non-null and has the given mode and return the
// appropriate response code (i.e., if null, there's a response code for that,
// if a reader and I want a writer, there's a code for that, etc.)
//
// All possible return values:
// ctx is null =>
SZ_HIDDEN
sz_response_t
sz_check_context(const sz_context_t *ctx, sz_mode_t mode);


#endif /* end __SZ_SNOWBALL__CONTEXT_HH__ include guard */
