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

