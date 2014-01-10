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

#include "context.hh"
#include "write_context.hh"
#include "read_context.hh"
#include "error_strings.hh"


s_sz_context::s_sz_context(sz_allocator_t *alloc)
: error(sz_errstr_no_error)
, ctx_alloc(alloc)
, stream(NULL)
, stream_pos(0)
{
  // nop
}


s_sz_context::~s_sz_context()
{
  // nop
}


sz_response_t
s_sz_context::file_error() const
{
  if (sz_stream_eof(stream)) {
    error = sz_errstr_eof;
    return SZ_ERROR_EOF;
  } else {
    if (mode() == SZ_READER) {
      error = sz_errstr_cannot_read;
      return SZ_ERROR_CANNOT_READ;
    } else {
      error = sz_errstr_cannot_write;
      return SZ_ERROR_CANNOT_WRITE;
    }
  }
}


sz_response_t
s_sz_context::set_stream(sz_stream_t *stream)
{
  if (stream == NULL) {
    error = sz_errstr_null_stream;
    return SZ_ERROR_INVALID_STREAM;
  } else if (opened()) {
    error = sz_errstr_open_set_stream;
    return SZ_ERROR_CONTEXT_OPEN;
  }

  this->stream = stream;
  stream_pos = sz_stream_tell(stream);

  return SZ_SUCCESS;
}


sz_response_t
sz_check_context(const sz_context_t *ctx, sz_mode_t mode)
{
  if (NULL == ctx) {
    return SZ_ERROR_NULL_CONTEXT;
  }

  if (mode != ctx->mode()) {
    ctx->error =
      (mode == SZ_READER)
      ? sz_errstr_read_on_write
      : sz_errstr_write_on_read;

    return SZ_ERROR_INVALID_OPERATION;
  }

  return SZ_SUCCESS;
}


SZ_DEF_BEGIN


const char *
sz_get_error(sz_context_t *ctx)
{
  return ctx ? ctx->error : NULL;
}


sz_response_t
sz_set_stream(sz_context_t *ctx, sz_stream_t *stream)
{
  return ctx ? ctx->set_stream(stream) : SZ_ERROR_NULL_CONTEXT;
}


sz_context_t *
sz_new_context(sz_mode_t mode, sz_allocator_t *allocator)
{
  if (!allocator) {
    allocator = sz_default_allocator();
  }

  void *memory = NULL;
  switch (mode) {
  case SZ_READER: {
    memory = sz_malloc(sizeof(sz_read_context_t), allocator);
    new (memory) sz_read_context_t(allocator);
  } break;

  case SZ_WRITER: {
    memory = sz_malloc(sizeof(sz_write_context_t), allocator);
    new (memory) sz_write_context_t(allocator);
  } break;

  default: break;
  }

  return (sz_context_t *)memory;
}


sz_response_t
sz_destroy_context(sz_context_t *ctx)
{
  if (ctx == NULL) {
    return SZ_ERROR_NULL_CONTEXT;
  }

  sz_allocator_t *alloc = ctx->ctx_alloc;
  ctx->~sz_context_t();
  sz_free(ctx, alloc);

  return SZ_SUCCESS;
}


sz_response_t
sz_close(sz_context_t *ctx)
{
  if (ctx == NULL) {
    return SZ_ERROR_NULL_CONTEXT;
  }

  return ctx->close();
}


sz_response_t
sz_open(sz_context_t *ctx)
{
  if (ctx == NULL) {
    return SZ_ERROR_NULL_CONTEXT;
  }

  return ctx->open();
}


SZ_DEF_END
