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
#include "utilities.hh"
#include "bufstream.hh"


// Writes an arbitrary type val to a stream and returns zero on success, or
// non-zero on failure. Should only be used for small-ish POD types.
template <typename T>
static
int sz_write_prim(sz_stream_t *stream, T val)
{
  return sz_stream_write(&val, sizeof(val), stream) != sizeof(val);
}


sz_write_context_t::sz_write_context_t(sz_allocator_t *alloc)
: s_sz_context(alloc)
, streams()
, compound_streams()
{
  /* nop */
}


sz_write_context_t::~sz_write_context_t()
{
  for (sz_stream_t *stream : compound_streams) {
    sz_stream_close(stream);
  }
}


sz_mode_t
sz_write_context_t::mode() const
{
  return SZ_WRITER;
}


void
sz_write_context_t::push_stack()
{
  streams.push_back(stream);
  stream = compound_streams.back();
}


void
sz_write_context_t::pop_stack()
{
  stream = streams.back();
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
  if (   sz_write_prim(stream, header.kind)
      || sz_write_prim(stream, header.name)
      || sz_write_prim(stream, header.size)) {
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
  sz_header_t header = {
    type,
    name,
    uint32_t(sizeof(sz_header_t) + type_size)
  };

  sz_response_t response = write_header(header);
  if (response != SZ_SUCCESS) {
    return response;
  }

  if (sz_stream_write(input, type_size, stream) != type_size) {
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

  if (input == NULL) {
    return write_null_pointer(name);
  }

  sz_response_t response = write_header(header.base);
  if (response != SZ_SUCCESS) {
    return response;
  }

  if (   sz_write_prim(stream, header.length)
      || sz_write_prim(stream, header.type)) {
    return file_error();
  }

  if (sz_stream_write(input, data_size, stream) != data_size) {
    return file_error();
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_write_context_t::write_null_pointer(uint32_t name)
{
  return write_header({ SZ_NULL_POINTER_CHUNK, name, sizeof(sz_header_t) });
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

  sz_stream_t *active = stream;

  sz_array_t header = {
    {
      SZ_ARRAY_CHUNK,
      name,
      uint32_t(sizeof(header) + sizeof(uint32_t) * length)
    },
    uint32_t(length),
    SZ_COMPOUND_REF_CHUNK
  };

  sz_response_t response = write_header(header.base);
  if (response != SZ_SUCCESS) {
    return response;
  }

  if (   sz_write_prim(stream, header.length)
      || sz_write_prim(stream, header.type)) {
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
  SZ_AS_WRITER(ctx, return)->write_compound_array(compounds, length, writer, writer_ctx, name);
}


sz_response_t
sz_write_bytes(
  sz_context_t *ctx,
  uint32_t name,
  const void *values,
  size_t length
  )
{
  SZ_AS_WRITER(ctx, return)->write_primitive(values, SZ_BYTES_CHUNK, length, name);
}


sz_response_t
sz_write_float(sz_context_t *ctx, uint32_t name, float value)
{
  SZ_AS_WRITER(ctx, return)->write_primitive(&value, SZ_FLOAT_CHUNK, sizeof(value), name);
}


sz_response_t
sz_write_floats(
  sz_context_t *ctx,
  uint32_t name,
  float *values,
  size_t length
  )
{
  SZ_AS_WRITER(ctx, return)->write_primitive_array(values, SZ_FLOAT_CHUNK, sizeof(*values), length, name);
}


sz_response_t
sz_write_int(sz_context_t *ctx, uint32_t name, int32_t value)
{
  SZ_AS_WRITER(ctx, return)->write_primitive(&value, SZ_SINT32_CHUNK, sizeof(value), name);
}


sz_response_t
sz_write_ints(
  sz_context_t *ctx,
  uint32_t name,
  int32_t *values,
  size_t length
  )
{
  SZ_AS_WRITER(ctx, return)->write_primitive_array(values, SZ_SINT32_CHUNK, sizeof(*values), length, name);
}


sz_response_t
sz_write_unsigned_int(sz_context_t *ctx, uint32_t name, uint32_t value)
{
  SZ_AS_WRITER(ctx, return)->write_primitive(&value, SZ_UINT32_CHUNK, sizeof(value), name);
}


sz_response_t
sz_write_unsigned_ints(
  sz_context_t *ctx,
  uint32_t name,
  uint32_t *values,
  size_t length
  )
{
  SZ_AS_WRITER(ctx, return)->write_primitive_array(values, SZ_UINT32_CHUNK, sizeof(*values), length, name);
}


SZ_DEF_END

