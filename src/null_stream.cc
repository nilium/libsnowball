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


static
size_t
sz_nullstream_read(void *p, size_t length, sz_stream_t *stream)
{
  (void)p;
  (void)length;
  (void)stream;
  return 0;
}


static
size_t
sz_nullstream_write(const void *p, size_t length, sz_stream_t *stream)
{
  (void)p;
  (void)length;
  (void)stream;
  return 0;
}


static
off_t
sz_nullstream_seek(off_t off, int whence, sz_stream_t *stream)
{
  (void)off;
  (void)whence;
  (void)stream;
  return 0;
}


static
int
sz_nullstream_eof(sz_stream_t *stream)
{
  (void)stream;
  return 1;
}


static
void
sz_nullstream_close(sz_stream_t *stream)
{
  (void)stream;
}


static sz_stream_t sz_null_stream_ops = {
  sz_nullstream_read,
  sz_nullstream_write,
  sz_nullstream_seek,
  sz_nullstream_eof,
  sz_nullstream_close
};


sz_stream_t *
sz_stream_null()
{
  return &sz_null_stream_ops;
}

