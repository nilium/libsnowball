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

#include "bufstream.hh"

#include <sstream>


typedef std::basic_stringstream<
#if __cplusplus >= 201103L
  char,
  std::char_traits<char>,
  sz_cxx_allocator_t<char>
#else
  char,
  std::char_traits<char>
#endif
  > sz_buffer_t;


static
size_t
sz_bufstream_read(void *out, size_t length, sz_stream_t *stream);


static
size_t
sz_bufstream_write(const void *in, size_t length, sz_stream_t *stream);


static
off_t
sz_bufstream_seek(off_t off, int whence, sz_stream_t *stream);


static
int
sz_bufstream_eof(sz_stream_t *stream);


static
void
sz_bufstream_close(sz_stream_t *stream);


struct SZ_HIDDEN sz_bufstream_t
{
  sz_stream_t base;
  sz_allocator_t *alloc;
  sz_mode_t mode;
  uint8_t opaque[sizeof(sz_buffer_t)];

  sz_buffer_t *buffer()
  {
    return (sz_buffer_t *)&opaque[0];
  }

  void init()
  {
    new (opaque) sz_buffer_t(
    #if __cplusplus >= 201103L
      sz_bufstring_t(sz_cxx_allocator_t<char>(alloc))
    #endif
      );
  }

  void finalize()
  {
    buffer()->~sz_buffer_t();
  }
};


static sz_stream_t sz_bufstream_base = {
  sz_bufstream_read,
  sz_bufstream_write,
  sz_bufstream_seek,
  sz_bufstream_eof,
  sz_bufstream_close
};


sz_stream_t *
sz_buffer_stream(sz_mode_t mode, sz_allocator_t *alloc)
{
  sz_bufstream_t *stream = (sz_bufstream_t *)sz_malloc(sizeof(sz_bufstream_t), alloc);
  stream->base = sz_bufstream_base;
  stream->init();
  stream->mode = mode;
  return (sz_stream_t *)stream;
}


sz_bufstring_t
sz_buffer_stream_data(sz_stream_t *stream)
{
  return ((sz_bufstream_t *)stream)->buffer()->str();
}


static
size_t
sz_bufstream_read(void *out, size_t length, sz_stream_t *stream)
{
  sz_bufstream_t *bufstream = (sz_bufstream_t *)stream;
  if (bufstream->mode == SZ_READER) {
    bufstream->buffer()->read((char *)out, length);
    return bufstream->buffer()->gcount();
  }
  return 0;
}


static
size_t
sz_bufstream_write(const void *in, size_t length, sz_stream_t *stream)
{
  sz_bufstream_t *bufstream = (sz_bufstream_t *)stream;
  if (bufstream->mode == SZ_WRITER) {
    off_t offset = bufstream->buffer()->tellp();
    bufstream->buffer()->write((const char *)in, length);
    return size_t(bufstream->buffer()->tellp() - offset);
  }
  return 0;
}


static
off_t
sz_bufstream_seek(off_t off, int whence, sz_stream_t *stream)
{
  sz_bufstream_t *bufstream = (sz_bufstream_t *)stream;
  sz_buffer_t::seekdir dir = sz_buffer_t::beg;
  switch (whence) {
  case SEEK_CUR: dir = sz_buffer_t::cur; break;
  case SEEK_END: dir = sz_buffer_t::end; break;
  case SEEK_SET:
  default: break;
  }

  if (bufstream->mode == SZ_WRITER) {
    bufstream->buffer()->seekp(off, dir);
    return bufstream->buffer()->tellp();
  } else {
    bufstream->buffer()->seekg(off, dir);
    return bufstream->buffer()->tellg();
  }
}


static
int
sz_bufstream_eof(sz_stream_t *stream)
{
  return ((sz_bufstream_t *)stream)->buffer()->eof();
}


static
void
sz_bufstream_close(sz_stream_t *stream)
{
  sz_bufstream_t *bufstream = (sz_bufstream_t *)stream;
  bufstream->finalize();
  sz_free(bufstream, bufstream->alloc);
}
