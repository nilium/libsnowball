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

#ifndef __UTILITIES_HH__
#define __UTILITIES_HH__


#define SZ_RETURN_IF(EXPR, RET) \
  do { \
    if ((EXPR)) { \
      return (RET); \
    } \
  } while(0)


#define SZ_RETURN_IF_ERROR(EXPR) \
  do { \
    sz_response_t sz_ret_error = (EXPR); \
    SZ_RETURN_IF(sz_ret_error != SZ_SUCCESS, sz_ret_error); \
  } while (0)


#define SZ_AS_WRITER(CTX, PREFIX) \
  SZ_RETURN_IF_ERROR(sz_check_context((CTX), SZ_WRITER)); \
  PREFIX (static_cast<sz_write_context_t *>((CTX)))


#define SZ_AS_READER(CTX, PREFIX) \
  SZ_RETURN_IF_ERROR(sz_check_context((CTX), SZ_WRITER)); \
  PREFIX (static_cast<sz_read_context_t *>((CTX)))


#endif /* end __UTILITIES_HH__ include guard */
