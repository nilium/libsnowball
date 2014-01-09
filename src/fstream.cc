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

#include <cstdio>


struct sz_fstream_t
{
  sz_stream_t base;

  FILE *file;
};


static
size_t
sz_fstream_read(void *out, size_t length, sz_stream_t *stream);


static
size_t
sz_fstream_write(const void *in, size_t length, sz_stream_t *stream);


static
off_t
sz_fstream_seek(off_t off, int whence, sz_stream_t *stream);


static
int
sz_fstream_eof(sz_stream_t *stream);


static
void
sz_fstream_close(sz_stream_t *stream);




sz_stream_t sz_fstream_base = {
  sz_fstream_read,
  sz_fstream_write,
  sz_fstream_seek,
  sz_fstream_eof,
  sz_fstream_close
};


SZ_DEF_BEGIN


sz_stream_t *
sz_stream_fopen(const char *filename, sz_mode_t mode)
{
  sz_fstream_t *stream = NULL;
  FILE *file = NULL;

  switch (mode) {
  case SZ_READER: file = fopen(filename, "rb"); break;
  case SZ_WRITER: file = fopen(filename, "wb"); break;
  default: break;
  }

  if (file) {
    stream = new sz_fstream_t;
    stream->base = sz_fstream_base;
    stream->file = file;
  }

  return (sz_stream_t *)stream;
}


SZ_DEF_END


static
size_t
sz_fstream_read(void *out, size_t length, sz_stream_t *stream)
{
  return 0;
}


static
size_t
sz_fstream_write(const void *in, size_t length, sz_stream_t *stream)
{
  return 0;
}


static
off_t
sz_fstream_seek(off_t off, int whence, sz_stream_t *stream)
{
  return 0;
}


static
int
sz_fstream_eof(sz_stream_t *stream)
{
  return 0;
}


static
void
sz_fstream_close(sz_stream_t *stream)
{
}

