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


SZ_DEF_BEGIN


size_t
sz_stream_read(void *out, size_t length, sz_stream_t *stream)
{
  if (stream) {
    return stream->read(out, length, stream);
  } else {
    return 0;
  }
}


size_t
sz_stream_write(const void *in, size_t length, sz_stream_t *stream)
{
  if (stream) {
    return stream->write(in, length, stream);
  } else {
    return 0;
  }
}


off_t
sz_stream_seek(off_t off, int whence, sz_stream_t *stream)
{
  if (stream) {
    return stream->seek(off, whence, stream);
  } else {
    return 0;
  }
}


off_t
sz_stream_tell(sz_stream_t *stream)
{
  return sz_stream_seek(0, SEEK_CUR, stream);
}


int
sz_stream_eof(sz_stream_t *stream)
{
  if (stream) {
    return stream->eof(stream);
  } else {
    return 0;
  }
}


void
sz_stream_close(sz_stream_t *stream)
{
  if (stream) {
    stream->close(stream);
  }
}


SZ_DEF_END

