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

#include "write_context.hh"
#include "error_strings.hh"
#include "utilities.hh"
#include "bufstream.hh"


sz_write_context_t::void_comp_t sz_write_context_t::void_comp;


// Writes an arbitrary type val to a stream and returns false on success, or
// true on failure. Should only be used for small-ish POD types.
// For those wondering why false is the successful case, it's so you can just
// do `if (write) then_error;`
template <typename T>
static
bool
sz_write_prim(sz_stream_t *stream, T val)
{
  return sz_stream_write(&val, sizeof(val), stream) != sizeof(val);
}


sz_write_context_t::sz_write_context_t(sz_allocator_t *alloc)
: s_sz_context(alloc)
, bufstream(NULL)
, active(NULL)
, streams(stream_stack_alloc_t(alloc))
, compound_streams(stream_stack_alloc_t(alloc))
, compound_indices(void_comp, compound_map_alloc_t(alloc))
{
  /* nop */
}


sz_write_context_t::~sz_write_context_t()
{
  cleanup();
}


sz_mode_t
sz_write_context_t::mode() const
{
  return SZ_WRITER;
}


sz_response_t
sz_write_context_t::open()
{
  if (active || bufstream) {
    error = sz_errstr_already_open;
    return SZ_ERROR_CONTEXT_OPEN;
  } else if (stream == NULL) {
    error = sz_errstr_null_stream;
    return SZ_ERROR_INVALID_STREAM;
  }

  active = bufstream = sz_buffer_stream(SZ_WRITER, ctx_alloc);
  return SZ_SUCCESS;
}


sz_response_t
sz_write_context_t::flush()
{
  typedef std::vector<
    sz_bufstring_t,
    sz_cxx_allocator_t<sz_bufstring_t>
    > compound_buffers_t;

  sz_root_t root = {
    SZ_MAGIC,
    0,
    uint32_t(compound_streams.size()),
    uint32_t(sizeof(root)),
    ~0U,
    ~0U
  };

  const sz_bufstring_t main_buf = sz_buffer_stream_data(bufstream);
  const size_t data_size = main_buf.size();

  uint32_t compounds_size = 0;
  const uint32_t mappings_size =
    root.num_compounds * uint32_t(sizeof(uint32_t));

  // Grab all compound buffers and their combined size
  compound_buffers_t compound_buffers(
    (sz_cxx_allocator_t<sz_bufstring_t>(ctx_alloc))
    );

  #if __cplusplus >= 201103L
  for (sz_stream_t *cmp_stream : compound_streams) {
  #else
  stream_stack_t::iterator iter = compound_streams.begin();
  const stream_stack_t::iterator end = compound_streams.end();
  for (; iter != end; ++iter) {
    sz_stream_t *cmp_stream = *iter;
  #endif
    compound_buffers.push_back(sz_buffer_stream_data(cmp_stream));
    compounds_size += compound_buffers.back().size();
  }

  root.compounds_offset = root.mappings_offset + mappings_size;
  root.data_offset = root.compounds_offset + compounds_size;
  root.size = root.data_offset + data_size;

  SZ_RETURN_IF_ERROR( write_root(root) );

  uint32_t relative_offset = uint32_t(sizeof(root) + mappings_size);
  #if __cplusplus >= 201103L
  for (const sz_bufstring_t &buf : compound_buffers) {
  #else
  compound_buffers_t::iterator cmpbuf_iter = compound_buffers.begin();
  compound_buffers_t::iterator cmpbuf_end = compound_buffers.end();
  for (; cmpbuf_iter != cmpbuf_end; ++cmpbuf_iter) {
    const sz_bufstring_t &buf = *cmpbuf_iter;
  #endif
    if (sz_write_prim(stream, relative_offset)) {
      return file_error();
    }
    relative_offset += uint32_t(buf.size());
  }

  #if __cplusplus >= 201103L
  for (const sz_bufstring_t &buf : compound_buffers) {
  #else
  cmpbuf_iter = compound_buffers.begin();
  cmpbuf_end = compound_buffers.end();
  for (; cmpbuf_iter != cmpbuf_end; ++cmpbuf_iter) {
    const sz_bufstring_t &buf = *cmpbuf_iter;
  #endif
    const size_t write_size = buf.size();
    if (sz_stream_write(buf.data(), write_size, stream) != write_size) {
      return file_error();
    }
  }

  if (   data_size
      && sz_stream_write(main_buf.data(), data_size, stream) != data_size) {
    return file_error();
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_write_context_t::close()
{
  SZ_RETURN_IF_CLOSED;

  SZ_RETURN_IF_ERROR( flush() );

  cleanup();

  return SZ_SUCCESS;
}


void
sz_write_context_t::cleanup()
{
  if (bufstream) {
    sz_stream_close(bufstream);
  }
  active = bufstream = NULL;

  #if __cplusplus >= 201103L
  for (sz_stream_t *cmp_stream : compound_streams) {
  #else
  stream_stack_t::iterator iter = compound_streams.begin();
  const stream_stack_t::iterator end = compound_streams.end();
  for (; iter != end; ++iter) {
    sz_stream_t *cmp_stream = *iter;
  #endif
    sz_stream_close(cmp_stream);
  }

  compound_indices.clear();
  compound_streams.clear();
  streams.clear();
}


void
sz_write_context_t::push_stack()
{
  streams.push_back(active);
  active = compound_streams.back();
}


void
sz_write_context_t::pop_stack()
{
  active = streams.back();
  streams.pop_back();
}


sz_response_t
sz_write_context_t::write_root(const sz_root_t &root)
{
  if (   sz_write_prim(stream, root.magic)
      || sz_write_prim(stream, root.size)
      || sz_write_prim(stream, root.num_compounds)
      || sz_write_prim(stream, root.mappings_offset)
      || sz_write_prim(stream, root.compounds_offset)
      || sz_write_prim(stream, root.data_offset)) {
    return file_error();
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_write_context_t::write_header(const sz_header_t &header)
{
  if (   sz_write_prim(active, header.kind)
      || sz_write_prim(active, header.name)
      || sz_write_prim(active, header.size)) {
    return file_error();
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_write_context_t::write_primitive(
  const void *input,
  sz_chunk_id_t type,
  size_t type_size,
  uint32_t name
  )
{
  SZ_RETURN_IF_CLOSED;

  sz_header_t header = {
    type,
    name,
    uint32_t(sizeof(sz_header_t) + type_size)
  };

  SZ_RETURN_IF_ERROR( write_header(header) );

  if (sz_stream_write(input, type_size, active) != type_size) {
    return file_error();
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_write_context_t::write_primitive_array(
  const void *input,
  sz_chunk_id_t type,
  size_t type_size,
  size_t length,
  uint32_t name
  )
{
  SZ_RETURN_IF_CLOSED;

  size_t data_size = length * type_size;
  sz_array_t header = {
    {
      SZ_ARRAY_CHUNK,
      name,
      uint32_t(sizeof(header) + data_size)
    },
    uint32_t(length),
    type
  };

  // null or zero-length arrays are written as a null chunk.
  if (input == NULL || !(length * type_size)) {
    return write_null_pointer(name);
  }

  SZ_RETURN_IF_ERROR( write_header(header.base) );

  if (   sz_write_prim(active, header.length)
      || sz_write_prim(active, header.type)
      || sz_stream_write(input, data_size, active) != data_size) {
    return file_error();
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_write_context_t::write_null_pointer(uint32_t name)
{
  const sz_header_t header = {
    SZ_NULL_POINTER_CHUNK,
    name,
    sizeof(sz_header_t)
  };
  return write_header(header);
}


uint32_t
sz_write_context_t::new_compound(void *compound)
{
  uint32_t index;
  sz_stream_t *bstream = sz_buffer_stream(SZ_WRITER, ctx_alloc);
  compound_streams.push_back(bstream);
  index = compound_streams.size();
  compound_indices.insert(compound_map_t::value_type(compound, index));
  return index;
}


uint32_t
sz_write_context_t::store_compound(
  void *compound,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx
  )
{
  uint32_t index;

  if (!compound) {
    return 0;
  }

  const compound_map_t::iterator found = compound_indices.find(compound);
  if (found != compound_indices.end()) {
    return found->second;
  }

  index = new_compound(compound);

  push_stack();
  writer(compound, this, writer_ctx);
  pop_stack();

  return index;
}


sz_response_t
sz_write_context_t::write_compound(
  void *compound,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx,
  uint32_t name
  )
{
  if (compound == NULL) {
    return write_null_pointer(name);
  }

  const uint32_t index = store_compound(compound, writer, writer_ctx);

  return write_primitive(&index, SZ_COMPOUND_REF_CHUNK, sizeof(index), name);
}


sz_response_t
sz_write_context_t::write_compound_array(
  void **compounds,
  size_t length,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx,
  uint32_t name
  )
{
  if (compounds == NULL) {
    return write_null_pointer(name);
  }

  sz_array_t header = {
    {
      SZ_ARRAY_CHUNK,
      name,
      uint32_t(sizeof(header) + sizeof(uint32_t) * length)
    },
    uint32_t(length),
    SZ_COMPOUND_REF_CHUNK
  };

  SZ_RETURN_IF_ERROR( write_header(header.base) );

  if (   sz_write_prim(active, header.length)
      || sz_write_prim(active, header.type)) {
    return file_error();
  }

  for (uint32_t index = 0; index < length; ++index) {
    const uint32_t ref = store_compound(compounds[index], writer, writer_ctx);
    if (sz_write_prim(active, ref)) {
      return file_error();
    }
  }

  return SZ_SUCCESS;
}


bool
sz_write_context_t::opened() const
{
  return bufstream || active;
}


SZ_DEF_BEGIN


sz_response_t
sz_write_compound(
  sz_context_t *ctx,
  uint32_t name,
  void *compound,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx)
{
  SZ_AS_WRITER(ctx, return)->write_compound(compound, writer, writer_ctx, name);
}


sz_response_t
sz_write_compounds(
  sz_context_t *ctx,
  uint32_t name,
  void **compounds,
  size_t length,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx
  )
{
  SZ_AS_WRITER(ctx, return)->write_compound_array(
    compounds,
    length,
    writer,
    writer_ctx,
    name
    );
}


sz_response_t
sz_write_bytes(
  sz_context_t *ctx,
  uint32_t name,
  const void *values,
  size_t length
  )
{
  SZ_AS_WRITER(ctx, return)->write_primitive(
    values,
    SZ_BYTES_CHUNK,
    length,
    name
    );
}


sz_response_t
sz_write_float(sz_context_t *ctx, uint32_t name, float value)
{
  SZ_AS_WRITER(ctx, return)->write_primitive(
    &value,
    SZ_FLOAT_CHUNK,
    sizeof(value),
    name
    );
}


sz_response_t
sz_write_floats(
  sz_context_t *ctx,
  uint32_t name,
  float *values,
  size_t length
  )
{
  SZ_AS_WRITER(ctx, return)->write_primitive_array(
    values,
    SZ_FLOAT_CHUNK,
    sizeof(*values),
    length,
    name
    );
}


sz_response_t
sz_write_int(sz_context_t *ctx, uint32_t name, int32_t value)
{
  SZ_AS_WRITER(ctx, return)->write_primitive(
    &value,
    SZ_SINT32_CHUNK,
    sizeof(value),
    name
    );
}


sz_response_t
sz_write_ints(
  sz_context_t *ctx,
  uint32_t name,
  int32_t *values,
  size_t length
  )
{
  SZ_AS_WRITER(ctx, return)->write_primitive_array(
    values,
    SZ_SINT32_CHUNK,
    sizeof(*values),
    length,
    name
    );
}


sz_response_t
sz_write_unsigned_int(sz_context_t *ctx, uint32_t name, uint32_t value)
{
  SZ_AS_WRITER(ctx, return)->write_primitive(
    &value,
    SZ_UINT32_CHUNK,
    sizeof(value),
    name
    );
}


sz_response_t
sz_write_unsigned_ints(
  sz_context_t *ctx,
  uint32_t name,
  uint32_t *values,
  size_t length
  )
{
  SZ_AS_WRITER(ctx, return)->write_primitive_array(
    values,
    SZ_UINT32_CHUNK,
    sizeof(*values),
    length,
    name
    );
}


SZ_DEF_END

