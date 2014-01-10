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

#ifndef __ERROR_STRINGS_HH__
#define __ERROR_STRINGS_HH__


#include <snowball.h>


SZ_HIDDEN extern const char *const sz_errstr_no_error;
SZ_HIDDEN extern const char *const sz_errstr_null_context;
SZ_HIDDEN extern const char *const sz_errstr_invalid_root;
SZ_HIDDEN extern const char *const sz_errstr_invalid_magic_head;
SZ_HIDDEN extern const char *const sz_errstr_invalid_magic_version;
SZ_HIDDEN extern const char *const sz_errstr_invalid_root;
SZ_HIDDEN extern const char *const sz_errstr_wrong_kind;
SZ_HIDDEN extern const char *const sz_errstr_bad_name;
SZ_HIDDEN extern const char *const sz_errstr_cannot_read;
SZ_HIDDEN extern const char *const sz_errstr_cannot_write;
SZ_HIDDEN extern const char *const sz_errstr_eof;
SZ_HIDDEN extern const char *const sz_errstr_write_on_read;
SZ_HIDDEN extern const char *const sz_errstr_read_on_write;
SZ_HIDDEN extern const char *const sz_errstr_compound_reader_null;
SZ_HIDDEN extern const char *const sz_errstr_already_closed;
SZ_HIDDEN extern const char *const sz_errstr_already_open;
SZ_HIDDEN extern const char *const sz_errstr_open_set_stream;
SZ_HIDDEN extern const char *const sz_errstr_null_stream;
SZ_HIDDEN extern const char *const sz_errstr_empty_array;
SZ_HIDDEN extern const char *const sz_errstr_nomem;


#endif /* end __ERROR_STRINGS_HH__ include guard */
