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

#include "read_context.hh"
#include "error_strings.hh"
#include "utilities.hh"


// Reads a primitive of type T to out (may be nullptr, though in that case
// you'll want to provide T yourself). Returns false on success, true if an
// error occurred.
template <typename T>
SZ_HIDDEN
bool
sz_read_prim(sz_stream_t *stream, T *out)
{
  T result = T();

  if (sz_stream_read(&result, sizeof(result), stream) == sizeof(result)) {

#if SZ_ENDIANNESS != SZ_BASE_ENDIANNESS
    T swapout = T(result);
    uint8_t *out = (uint8_t *)&result;
    const uint8_t *in = (const uint8_t *)&swapout;
    for (size_t i = 0; i < sizeof(T); ++i) {
      out[(sizeof(T) - 1) - i] = in[i];
    }
#endif

    if (out) {
      *out = result;
    }

    return false;
  }

  return true;
}


#if SZ_ENDIANNESS != SZ_BASE_ENDIANNESS

template <>
SZ_HIDDEN
bool
sz_read_prim(sz_stream_t *stream, int8_t *out)
{
  int8_t result = 0;

  if (sz_stream_read(&result, sizeof(result), stream) == sizeof(result)) {
    if (out) {
      *out = result;
    }

    return false;
  }

  return true;
}

template <>
SZ_HIDDEN
bool
sz_read_prim(sz_stream_t *stream, uint8_t *out)
{
  uint8_t result = 0;

  if (sz_stream_read(&result, sizeof(result), stream) == sizeof(result)) {
    if (out) {
      *out = result;
    }

    return false;
  }

  return true;
}


template <>
SZ_HIDDEN
bool
sz_read_prim(sz_stream_t *stream, uint32_t *out)
{
  uint32_t result = 0;

  if (sz_stream_read(&result, sizeof(result), stream) == sizeof(result)) {
    if (out) {
      *out = sz_htonl(result);
    }

    return false;
  }

  return true;
}


template <>
SZ_HIDDEN
bool
sz_read_prim(sz_stream_t *stream, int32_t *out)
{
  uint32_t result = 0;
  bool r = sz_read_prim(stream, &result);
  if (r && out) {
    *out = *(int32_t *)&result;
  }
  return r;
}


template <>
SZ_HIDDEN
bool
sz_read_prim(sz_stream_t *stream, float *out)
{
  uint32_t result = 0;
  bool r = sz_read_prim(stream, &result);
  if (r && out) {
    *out = *(float *)&result;
  }
  return r;
}

#endif


const sz_read_context_t::unpacked_compound_t
sz_read_context_t::default_unpacked_compound = {
  0,     // position
  NULL,  // value
  false  // unpacked
};


sz_read_context_t::sz_read_context_t(sz_allocator_t *alloc)
: s_sz_context(alloc)
, compounds(sz_cxx_allocator_t<unpacked_compound_t>(alloc))
, offsets(sz_cxx_allocator_t<off_t>(alloc))
, is_open(false)
{
  /* nop */
}


sz_read_context_t::~sz_read_context_t()
{
  /* nop */
}


sz_mode_t
sz_read_context_t::mode() const
{
  return SZ_READER;
}


// Roots
sz_response_t
sz_read_context_t::read_root(sz_root_t *root)
{
  sz_root_t res;
  if (   sz_read_prim(stream, &res.magic)
      || sz_read_prim(stream, &res.size)
      || sz_read_prim(stream, &res.num_compounds)
      || sz_read_prim(stream, &res.mappings_offset)
      || sz_read_prim(stream, &res.compounds_offset)
      || sz_read_prim(stream, &res.data_offset)) {
    return file_error();
  }

  if (res.magic != SZ_MAGIC) {
    // Check if the version is supported. Currently, this just means the version
    // number is the same or less and the first bytes of the stream are 'SZ'
    if ((res.magic & 0xFFFF) != (SZ_MAGIC & 0xFFFF)) {
      error = sz_errstr_invalid_magic_head;
      return SZ_ERROR_MALFORMED_MAGIC_HEAD;
    }

    if (SZ_MAGIC_VER_INT(res.magic) > SZ_MAGIC_VER_INT(SZ_MAGIC)) {
      error = sz_errstr_invalid_magic_version;
      return SZ_ERROR_MALFORMED_MAGIC_VERSION;
    }
  }

  if (root) {
    *root = res;
  }

  return SZ_SUCCESS;
}


// Headers
sz_response_t
sz_read_context_t::read_header(
  sz_header_t *header,
  sz_chunk_id_t type,
  uint32_t name,
  bool null_allowed
  )
{
  sz_header_t res;

  if (   sz_read_prim(stream, &res.kind)
      || sz_read_prim(stream, &res.name)
      || sz_read_prim(stream, &res.size)) {
    return file_error();
  }

  if (header) {
    *header = res;
  }

  if (   (res.kind == SZ_NULL_POINTER_CHUNK && !null_allowed)
      || res.kind != type) {
    error = sz_errstr_wrong_kind;
    return SZ_ERROR_WRONG_KIND;
  } else if (res.name != name) {
    error = sz_errstr_bad_name;
    return SZ_ERROR_BAD_NAME;
  }

  return SZ_SUCCESS;
}


// Arrays
sz_response_t
sz_read_context_t::read_array_header(
  sz_array_t *chunk,
  sz_chunk_id_t type,
  uint32_t name
  )
{
  sz_array_t res;

  SZ_RETURN_IF_ERROR( read_header(&res.base, SZ_ARRAY_CHUNK, name, true) );

  if (res.base.kind == SZ_NULL_POINTER_CHUNK) {
    return SZ_SUCCESS;
  } else if (   sz_read_prim(stream, &res.length)
             || sz_read_prim(stream, &res.type)) {
    return file_error();
  }

  if (chunk) {
    *chunk = res;
  }

  if (res.type == type) {
    return SZ_SUCCESS;
  } else {
    return SZ_ERROR_WRONG_KIND;
  }
}


sz_response_t
sz_read_context_t::read_array_body(
  void **buf_out,
  size_t *length,
  const sz_array_t *chunk,
  sz_allocator_t *alloc
  )
{
  const bool have_buffer = (buf_out != nullptr) && (*buf_out != nullptr);
  void *buffer;

  if (chunk->base.kind == SZ_NULL_POINTER_CHUNK) {
    if (buf_out) {
      *buf_out = NULL;
    }

    if (length) {
      *length = 0;
    }

    return SZ_SUCCESS;
  }

  const size_t arr_length = size_t(chunk->length);

  if (arr_length == 0) {
    // Note: this is technically recoverable.
    error = sz_errstr_empty_array;
    return SZ_ERROR_EMPTY_ARRAY;
  }

  if (length) {
    *length = arr_length;
  }

  const size_t block_remainder = size_t(chunk->base.size) - sizeof(sz_array_t);

  off_t end_of_block = sz_stream_tell(stream);

  if (end_of_block == -1) {
    return file_error();
  }

  end_of_block += block_remainder;

  sz_response_t response = SZ_SUCCESS;

  if (buf_out) {
    if (have_buffer) {
      buffer = *buf_out;
    } else {
      buffer = sz_malloc(block_remainder, alloc);

      if (!buffer) {
        error = sz_errstr_nomem;
        response = SZ_ERROR_OUT_OF_MEMORY;
        goto sz_read_array_body_done;
      }
    }

    if (sz_stream_read(buffer, block_remainder, stream) != block_remainder) {
      if (!have_buffer) {
        sz_free(buffer, alloc);
      }
      response = file_error();
      goto sz_read_array_body_done;
    }

#if SZ_ENDIANNESS != SZ_BASE_ENDIANNESS
    switch (chunk->type) {
    case SZ_UINT32_CHUNK:
    case SZ_SINT32_CHUNK:
    case SZ_FLOAT_CHUNK:
    case SZ_COMPOUND_REF_CHUNK: {
      uint32_t *data = (uint32_t *)buffer;
      const uint32_t *const data_end = data + arr_length;
      for (; data < data_end; ++data) {
        *data = sz_htonl(*data);
      }
    } break;

    default:
      if (!have_buffer) {
        sz_free(buffer, alloc);
      }
      response = SZ_ERROR_INVALID_OPERATION;
      error = sz_errstr_wrong_kind;
      goto sz_read_array_body_done;
    }
#endif

    if (!have_buffer) {
      *buf_out = buffer;
    }
  } else {
    error = sz_errstr_cannot_read;
    response = SZ_ERROR_CANNOT_READ;
  }

sz_read_array_body_done:
  sz_stream_seek(end_of_block, SEEK_SET, stream);

  return response;
}


sz_response_t
sz_read_context_t::read_primitive_array(
  void **out,
  size_t *length,
  sz_chunk_id_t type,
  size_t type_size,
  uint32_t name,
  sz_allocator_t *buf_alloc
  )
{
  SZ_RETURN_IF_CLOSED;

  sz_response_t response;
  sz_array_t header;
  const off_t error_off = sz_stream_tell(stream);

  response = read_array_header(&header, type, name);
  if (response != SZ_SUCCESS) {
    goto sz_read_primitive_array_error;
  }

  response = read_array_body(out, length, &header, buf_alloc);
  if (response != SZ_SUCCESS) {
    goto sz_read_primitive_array_error;
  }

  return SZ_SUCCESS;

sz_read_primitive_array_error:
  sz_stream_seek(error_off, SEEK_SET, stream);
  return response;
}


sz_response_t
sz_read_context_t::read_bytes(
  void **out,
  size_t *length,
  uint32_t name,
  sz_allocator_t *buf_alloc
  )
{
  SZ_RETURN_IF_CLOSED;

  const bool have_buffer = (out != nullptr) && (*out != nullptr);
  sz_response_t response = SZ_SUCCESS;
  sz_header_t header;
  const off_t error_off = sz_stream_tell(stream);
  size_t bytes_length = 0;
  void *buffer = NULL;

  SZ_JUMP_IF_ERROR(
    read_header(&header, SZ_BYTES_CHUNK, name, true),
    response,
    sz_read_bytes_error
    );

  if (header.kind != SZ_NULL_POINTER_CHUNK) {
    bytes_length = header.size - sizeof(header);

    if (out) {
      if (have_buffer) {
        buffer = *out;
      } else {
        buffer = sz_malloc(bytes_length, buf_alloc);

        if (!buffer) {
          error = sz_errstr_nomem;
          response = SZ_ERROR_OUT_OF_MEMORY;
          goto sz_read_bytes_error;
        }
      }


      if (sz_stream_read(buffer, bytes_length, stream) != bytes_length) {
        if (!have_buffer) {
          sz_free(buffer, buf_alloc);
        }
        response = file_error();
        goto sz_read_bytes_error;
      }
    } else {
      sz_stream_seek(bytes_length, SEEK_CUR, stream);
    }
  }

  if (out) {
    *out = buffer;
  }

  if (length) {
    *length = bytes_length;
  }

  return SZ_SUCCESS;

  sz_read_bytes_error:
    sz_stream_seek(error_off, SEEK_SET, stream);
    return response;
}


// Info stack
void
sz_read_context_t::push_stack()
{
  offsets.push_back(sz_stream_tell(stream));
}


void
sz_read_context_t::pop_stack()
{
  sz_stream_seek(offsets.back(), SEEK_SET, stream);
  offsets.pop_back();
}


// Primitives
sz_response_t
sz_read_context_t::read_primitive(
  void *out,
  sz_chunk_id_t type,
  size_t type_size,
  uint32_t name
  )
{
  SZ_RETURN_IF_CLOSED;

  sz_header_t header;
  const off_t error_off = sz_stream_tell(stream);
  sz_response_t response = SZ_SUCCESS;

  response = read_header(&header, type, name, false);
  if (response != SZ_SUCCESS) {
    goto sz_read_primitive_error;
  }

  if (sz_stream_read(out, type_size, stream) != type_size) {
    response = file_error();
    goto sz_read_primitive_error;
  }

  // TODO: Verify structure size if changing header structure (i.e., if the
  // format changes, make the appropriate changes to support older versions and
  // account for padding bytes and so on)
  if (   header.kind != type
      || header.size != sizeof(sz_header_t) + type_size) {
    error = sz_errstr_wrong_kind;
    response = SZ_ERROR_WRONG_KIND;
    goto sz_read_primitive_error;
  } else if (header.name != name) {
    error = sz_errstr_bad_name;
    response = SZ_ERROR_BAD_NAME;
    goto sz_read_primitive_error;
  }

#if SZ_ENDIANNESS != SZ_BASE_ENDIANNESS
  if (out && type_size > 1) {
    switch (type_size) {
    case 2: *(uint16_t *)out = sz_htons(*(uint16_t *)out); break;
    case 4: *(uint32_t *)out = sz_htonl(*(uint32_t *)out); break;
    default:
      error = sz_errstr_wrong_kind;
      response = SZ_ERROR_INVALID_OPERATION;
      goto sz_read_primitive_error;
    }
  }
#endif

  return SZ_SUCCESS;

sz_read_primitive_error:
  sz_stream_seek(error_off, SEEK_SET, stream);
  return response;
}


sz_response_t
sz_read_context_t::read_compound(
  void **compound,
  uint32_t name,
  sz_compound_reader_fn_t reader,
  void *reader_ctx
  )
{
  const off_t error_off = sz_stream_tell(stream);
  sz_response_t response = SZ_SUCCESS;
  sz_header_t header;
  void *result = NULL;

  SZ_JUMP_IF_ERROR(
    read_header(&header, SZ_COMPOUND_REF_CHUNK, name, true),
    response,
    sz_read_compound_error
    );

  if (header.kind != SZ_NULL_POINTER_CHUNK) {
    uint32_t compound_index = 0;

    if (sz_read_prim(stream, &compound_index)) {
      response = file_error();
      goto sz_read_compound_error;
    }

    SZ_JUMP_IF_ERROR(
      get_compound(&result, compound_index, reader, reader_ctx),
      response,
      sz_read_compound_error
      );
  }

  if (compound) {
    *compound = result;
  }

  return SZ_SUCCESS;

sz_read_compound_error:
  sz_stream_seek(error_off, SEEK_SET, stream);
  return response;
}


sz_response_t
sz_read_context_t::read_compound_array(
  void ***compounds,
  size_t *length,
  uint32_t name,
  sz_compound_reader_fn_t reader,
  void *reader_ctx,
  sz_allocator_t *alloc
  )
{
  sz_response_t response;
  sz_array_t header;
  const off_t error_off = sz_stream_tell(stream);

  const bool have_buffer = (compounds != nullptr) && (*compounds != nullptr);
  void **compound_ptrs = NULL;
  size_t array_length = 0;

  SZ_JUMP_IF_ERROR(
    read_array_header(&header, SZ_COMPOUND_REF_CHUNK, name),
    response,
    sz_read_compound_array_error
    );


  if (header.base.kind == SZ_ARRAY_CHUNK) {
    array_length = header.length;

    if (compounds) {
      uint32_t index = 0; // for iterating while reading compound indices
      uint32_t compound_index = 0; // used with get_compound

      if (have_buffer) {
        compound_ptrs = *compounds;
      } else {
        compound_ptrs = (void **)sz_malloc(sizeof(void *) * header.length, alloc);

        if (compound_ptrs == NULL) {
          error = sz_errstr_nomem;
          response = SZ_ERROR_OUT_OF_MEMORY;
          goto sz_read_compound_array_error;
        }
      }

      for (; index < array_length; ++index) {
        if (sz_read_prim(stream, &compound_index)) {
          if (!have_buffer) {
            sz_free(compound_ptrs, alloc);
          }
          response = file_error();
          goto sz_read_compound_array_error;
        }

        SZ_JUMP_IF_ERROR(
          get_compound(
            &compound_ptrs[index],
            compound_index,
            reader,
            reader_ctx
            ),
          response,
          sz_read_compound_array_error
          );
      }
    } else {
      sz_stream_seek(sizeof(uint32_t) * header.length, SEEK_CUR, stream);
    }
  }

  if (length) {
    *length = array_length;
  }

  if (compounds) {
    *compounds = compound_ptrs;
  }

  return SZ_SUCCESS;

sz_read_compound_array_error:
  sz_stream_seek(error_off, SEEK_SET, stream);
  return response;
}


sz_response_t
sz_read_context_t::get_compound(
  void **out,
  uint32_t index,
  sz_compound_reader_fn_t reader,
  void *reader_ctx
  )
{
  if (index == 0) {
    error = sz_errstr_compound_zero;
    return SZ_ERROR_INVALID_OPERATION;
  }

  unpacked_compound_t &pack = compounds[index - 1];

  if (!pack.unpacked) {
    push_stack();
    sz_stream_seek(pack.offset, SEEK_SET, stream);

    sz_header_t header;
    sz_response_t response;
    response = read_header(&header, SZ_COMPOUND_CHUNK, index, false);
    if (response != SZ_SUCCESS) {
      pop_stack();
      return response;
    }

    pack.unpacked = true;
    reader(&pack.value, this, reader_ctx);
    pop_stack();
  }

  if (out) {
    *out = pack.value;
  }

  return SZ_SUCCESS;
}


// Open / close ops
sz_response_t
sz_read_context_t::open()
{
  if (opened()) {
    error = sz_errstr_already_open;
    return SZ_ERROR_CONTEXT_OPEN;
  } else if (stream == NULL) {
    error = sz_errstr_null_stream;
    return SZ_ERROR_INVALID_STREAM;
  }

  is_open = true;

  push_stack();

  sz_root_t root;
  SZ_RETURN_IF_ERROR( read_root(&root) );

  const off_t mappings_off = stream_pos + root.mappings_offset;
  const off_t compounds_off = stream_pos + root.compounds_offset;
  const off_t data_off = stream_pos + root.data_offset;

  compounds.resize(root.num_compounds, default_unpacked_compound);

  sz_stream_seek(mappings_off, SEEK_SET, stream);
  // Read compound offsets
  #if __cplusplus >= 201103L
  for (unpacked_compound_t &pack : compounds) {
  #else
  compounds_t::iterator iter = compounds.begin();
  const compounds_t::iterator end = compounds.end();
  for (; iter != end; ++iter) {
    unpacked_compound_t &pack = *iter;
  #endif
    uint32_t offset = 0;
    if (sz_read_prim(stream, &offset)) {
      pop_stack();
      return file_error();
    }

    pack.offset = compounds_off + off_t(offset);
  }

  pop_stack();

  // Jump to the data (we should already be there, but in case there's)
  sz_stream_seek(data_off, SEEK_SET, stream);

  SZ_RETURN_IF_ERROR( read_header(NULL, SZ_DATA_CHUNK, SZ_DATA_NAME, false) );

  return SZ_SUCCESS;
}


sz_response_t
sz_read_context_t::close()
{
  SZ_RETURN_IF_CLOSED;

  is_open = false;

  return SZ_SUCCESS;
}


bool
sz_read_context_t::opened() const
{
  return is_open;
}


SZ_DEF_BEGIN


sz_response_t
sz_read_compound(
  void **compound,
  sz_context_t *ctx,
  uint32_t name,
  sz_compound_reader_fn_t *reader,
  void *reader_ctx
  )
{
  SZ_AS_READER(ctx, return)
    ->read_compound(
      compound,
      name,
      reader,
      reader_ctx
      );
}


sz_response_t
sz_read_compounds(
  void ***compounds,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_compound_reader_fn_t *reader,
  void *reader_ctx,
  sz_allocator_t *buf_alloc
  )
{
  SZ_AS_READER(ctx, return)
    ->read_compound_array(
      compounds,
      length,
      name,
      reader,
      reader_ctx,
      buf_alloc
      );
}


sz_response_t
sz_read_bytes(
  void **out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_allocator_t *buf_alloc
  )
{
  SZ_AS_READER(ctx, return)->read_bytes(out, length, name, buf_alloc);
}


sz_response_t
sz_read_float(float *out, sz_context_t *ctx, uint32_t name)
{
  float result;
  sz_response_t response;

  SZ_AS_READER(ctx, response = )
    ->read_primitive(&result, SZ_FLOAT_CHUNK, sizeof(result), name);
  SZ_RETURN_IF_ERROR(response);

  if (out) {
    *out = result;
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_read_floats(
  float **out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_allocator_t *buf_alloc
  )
{
  SZ_AS_READER(ctx, return)
    ->read_primitive_array(
      (void **)out,
      length,
      SZ_FLOAT_CHUNK,
      sizeof(**out),
      name,
      buf_alloc
      );
}


sz_response_t
sz_read_int(int32_t *out, sz_context_t *ctx, uint32_t name)
{
  int32_t result;
  sz_response_t response;

  SZ_AS_READER(ctx, response = )
    ->read_primitive(&result, SZ_SINT32_CHUNK, sizeof(result), name);
  SZ_RETURN_IF_ERROR(response);

  if (out) {
    *out = result;
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_read_ints(
  int32_t **out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_allocator_t *buf_alloc
  )
{

  SZ_AS_READER(ctx, return)
    ->read_primitive_array(
      (void **)out,
      length,
      SZ_SINT32_CHUNK,
      sizeof(**out),
      name,
      buf_alloc
      );
}


sz_response_t
sz_read_unsigned_int(uint32_t *out, sz_context_t *ctx, uint32_t name)
{
  uint32_t result;
  sz_response_t response;

  SZ_AS_READER(ctx, response = )
    ->read_primitive(&result, SZ_UINT32_CHUNK, sizeof(result), name);
  SZ_RETURN_IF_ERROR(response);

  if (out) {
    *out = result;
  }

  return SZ_SUCCESS;
}


sz_response_t
sz_read_unsigned_ints(
  uint32_t **out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_allocator_t *buf_alloc
  )
{

  SZ_AS_READER(ctx, return)
    ->read_primitive_array(
      (void **)out,
      length,
      SZ_UINT32_CHUNK,
      sizeof(**out),
      name,
      buf_alloc
      );
}


SZ_DEF_END

