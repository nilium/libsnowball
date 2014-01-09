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

#ifndef __SZ_SNOWBALL_H__
#define __SZ_SNOWBALL_H__

#if defined(__cplusplus)
# define SZ_DEF_BEGIN extern "C" {
# define SZ_DEF_END \
  }
  // Place the } on a second line because Sublime hates highlighting this.
# define SZ_TYPE_ENUM(TYPE) : TYPE
#else
# define SZ_DEF_BEGIN
# define SZ_DEF_END
# define SZ_TYPE_ENUM(TYPE)
#endif



SZ_DEF_BEGIN
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
SZ_DEF_END

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"

#if defined(__APPLE__)
# include <TargetConditionals.h>
#endif

/* define NULL ifndef */
#if !defined(NULL)
# if defined(__cplusplus)
#   if __cplusplus >= 201103L
#     define NULL (nullptr)
#   else
#     define NULL (0)
#   endif
# else /* defined(__cplusplus) */
#   define NULL ((void *)0)
# endif /* !__cplusplus */
#endif /* !defined(NULL) */

#define SZ_EXPORT __attribute__((visibility("default")))
#define SZ_HIDDEN __attribute__((visibility("hidden")))

#pragma GCC diagnostic pop

#define SZ_VERSION_MAJOR      1
#define SZ_VERSION_MINOR      0
#define SZ_VERSION_REVISION   0


SZ_DEF_BEGIN


enum SZ_TYPE_ENUM(uint32_t)
{
  SZ_MAGIC = 0x31305A53
};

#define SZ_MAGIC_VER_SUB_ONE(m) \
  (((m) & 0xFF) - 0x30)

#define SZ_MAGIC_VER_INT(m) \
  (SZ_MAGIC_VER_SUB_ONE((m) >> 24) + SZ_MAGIC_VER_SUB_ONE((m) >> 16) * 10)

typedef enum SZ_TYPE_ENUM(uint32_t)
{
  SZ_INVALID_CHUNK = 0,
  // Chunk types
  SZ_COMPOUND_CHUNK = 1,
  SZ_COMPOUND_REF_CHUNK = 2,
  SZ_FLOAT_CHUNK = 3,
  SZ_UINT32_CHUNK = 4,
  SZ_SINT32_CHUNK = 5,
  SZ_ARRAY_CHUNK = 6,
  SZ_BYTES_CHUNK = 7,
  // The null pointer chunk may substitute any compound, array, or bytes chunk
  SZ_NULL_POINTER_CHUNK = 8,
  SZ_DOUBLE_CHUNK = 9,
} sz_chunk_id_t;

// Responses
typedef enum
{
  SZ_SUCCESS = 0,
  SZ_ERROR_NONE = SZ_SUCCESS,
  // Root is invalid, typically meaning it's not actually a serializable stream.
  SZ_INVALID_ROOT,
  // Error when attempting to write an empty array (length == 0).
  SZ_ERROR_EMPTY_ARRAY,
  // A pointer is NULL.  For input operations, this means a required input
  // (e.g., an array of floats) is NULL.  For output operations, all pointers
  // other than the context are NULL when it's not permitted.  This may also
  // mean a required argument to a set_* function is NULL.
  SZ_ERROR_NULL_POINTER,
  // Exactly what it says on the tin.
  SZ_ERROR_NULL_CONTEXT,
  // The operation attempted cannot be performed.  This usually means
  // attempting to perform a read operation on a writable serializer.
  SZ_ERROR_INVALID_OPERATION,
  // The name being read is not the next name in the sequence
  SZ_ERROR_BAD_NAME,
  // What it says on the tin.  Again.
  SZ_ERROR_OUT_OF_MEMORY,
  // Attempted to read the wrong kind of data from the serializer.
  SZ_ERROR_WRONG_KIND,
  // Stream is somehow invalid
  SZ_ERROR_INVALID_STREAM,
  // Could not or cannot read from/write to the stream.
  SZ_ERROR_CANNOT_READ,
  SZ_ERROR_CANNOT_WRITE,
  // Could not or cannot write to a stream.
  // Reached EOF prematurely
  SZ_ERROR_EOF
} sz_response_t;

typedef enum
{
  SZ_READER,
  SZ_WRITER
} sz_mode_t;

typedef struct s_sz_context sz_context_t;

typedef struct s_sz_version
{
  int major;
  int minor;
  int revision;
} sz_version_t;

SZ_EXPORT sz_version_t sz_version();

typedef void (sz_compound_writer_fn_t)(
  void *compound,
  sz_context_t *ctx,
  void *writer_ctx
  );


/*
  An important note on how sz_compound_reader should work:
  When the reader is called, the output pointer, p, is only guaranteed to be
  valid before any other calls are made to the serializer. Second, the only way
  to deal with semi-circular references is to allocate, write p, and continue
  reading the contents of the compound only afterward.

  So, an example function might look like:

  sz_entity_reader(sz_context_t *ctx, void **p, void *rd_ctx)
  {
    char *name;
    entity_t *entity = entity_new(..);  // Allocate
    *p = entity;                        // Write to p
    // Do whatever else you need to, never access p again
    sz_read_bytes(ctx, KEY_ENTITY_NAME, &name);
    ...
  }
*/
typedef void (sz_compound_reader_fn_t)(
  void **compound,
  sz_context_t *ctx,
  void *reader_ctx);


/*
struct s_sz_context {
  sz_allocator_t alloc;

  const char *error;

  int mode;
  int open;
  int compound_level;

  sz_stream_t *stream;
  off_t stream_pos;

  sz_stream_t *active;

  // writing: map of compounds in use to their indices
  map_t compound_ptrs;
  // stack that operates differently when reading and writing
  // writing: stack of active buffers
  // reading: stack of off_t locations in the stream
  array_t stack;
  // compound pointers
  // writing: pointers to buffers of compounds
  // reading: pointers to file offsets of compounds and their unpacked pointers
  array_t compounds;
  // output buffer
  // unused in reading
  buffer_t buffer;
  sz_stream_t *buffer_stream;
};
*/


typedef struct s_sz_allocator sz_allocator_t;

struct s_sz_allocator
{
  void *(*malloc)(size_t sz, sz_allocator_t *allocator);
  void (*free)(void *ptr, sz_allocator_t *allocator);
};


typedef struct s_sz_stream sz_stream_t;


struct s_sz_stream
{
  // Reads length bytes from the stream into out and returns the number of bytes
  // actually read. Returning a number less than length is treated as an error,
  // though typically it just means EOF was reached before it was expected, in
  // which case the snowball may be malformed.
  size_t (*read)(void *out, size_t length, sz_stream_t *stream);

  // Writes length bytes from in to the stream, returns the number of bytes
  // actually written. If anything other than length is returned, snowball
  // assumes an error occurred. length 0 should be a no-op and ignored.
  size_t (*write)(const void *in, size_t length, sz_stream_t *stream);

  // Seeks to the given offset (only SEEK_CUR and SEEK_SET need to be
  // supported). Returns the resulting offset into the stream.
  off_t (*seek)(off_t off, int whence, sz_stream_t *stream);

  // Returns whether the stream is at its EOF (or whatever the equivalent is).
  int (*eof)(sz_stream_t *stream);

  // Disposes of the stream and deallocates it. Streams are considered invalid
  // after this and are free to crash or explode or do whatever they want if
  // accessed again.
  void (*close)(sz_stream_t *stream);
};



SZ_EXPORT
sz_stream_t *
sz_stream_fopen(const char *filename, sz_mode_t mode);

SZ_EXPORT
size_t
sz_stream_read(void *out, size_t length, sz_stream_t *stream);

SZ_EXPORT
size_t
sz_stream_write(const void *in, size_t length, sz_stream_t *stream);

SZ_EXPORT
off_t
sz_stream_seek(off_t off, int whence, sz_stream_t *stream);

// Same as seek(0, SEEK_CUR, stream)
SZ_EXPORT
off_t
sz_stream_tell(sz_stream_t *stream);

SZ_EXPORT
int
sz_stream_eof(sz_stream_t *stream);

SZ_EXPORT
int
sz_stream_close(sz_stream_t *stream);



/////////// malloc / free

// Returns a default allocator -- uses system malloc/free.
SZ_EXPORT
sz_allocator_t *
sz_default_allocator();

SZ_EXPORT
void
sz_free(void *ptr, sz_allocator_t *allocator);

SZ_EXPORT
void *
sz_malloc(size_t size, sz_allocator_t *allocator);


SZ_EXPORT
sz_context_t *
sz_new_context(sz_mode_t mode, sz_allocator_t *allocator);

SZ_EXPORT
sz_response_t
sz_destroy_context(sz_context_t *ctx);


// Done with setting up, starts de/serialization.
SZ_EXPORT
sz_response_t
sz_open(sz_context_t *ctx);

// Ends de/serialization and closes the context.
// If serializing, the serialized data will be written to the file.
SZ_EXPORT
sz_response_t
sz_close(sz_context_t *ctx);

/////////// Attributes (use before sz_open)

// Input / output file
SZ_EXPORT
sz_response_t
sz_set_stream(sz_context_t *ctx, sz_stream_t *stream);

// Returns a NULL-terminated error string. This pointer is managed by snowball
// and should not be freed or modified.
SZ_EXPORT
const char *
sz_get_error(sz_context_t *ctx);

//////////// Read/write operations


// Begins a compound for the pointer P
// Returns SZ_CONTINUE for
SZ_EXPORT
sz_response_t
sz_write_compound(
  sz_context_t *ctx,
  uint32_t name,
  void *compound,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx);

SZ_EXPORT
sz_response_t
sz_read_compound(
  sz_context_t *ctx,
  void **compound,
  uint32_t name,
  sz_compound_reader_fn_t *reader,
  void *reader_ctx
  );


SZ_EXPORT
sz_response_t
sz_write_compounds(
  sz_context_t *ctx,
  uint32_t name,
  void **out,
  size_t length,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx
  );

// Returns an array of pointers to the read compounds.
// The memory must be freed by the caller.
SZ_EXPORT
sz_response_t
sz_read_compounds(
  sz_context_t *ctx,
  uint32_t name,
  void ***p,
  size_t *length,
  sz_compound_reader_fn_t *reader,
  void *reader_ctx,
  sz_allocator_t *buf_alloc
  );


// Writes an array of bytes with the given length.
SZ_EXPORT
sz_response_t
sz_write_bytes(
  sz_context_t *ctx,
  uint32_t name,
  const void *values,
  size_t length
  );

// Reads an array of bytes and returns it via `out`.
// The memory must be freed by the caller.
SZ_EXPORT
sz_response_t
sz_read_bytes(
  sz_context_t *ctx,
  uint32_t name,
  void **out,
  size_t *length,
  sz_allocator_t *buf_alloc
  );


SZ_EXPORT
sz_response_t
sz_write_float(sz_context_t *ctx, uint32_t name, float value);

SZ_EXPORT
sz_response_t
sz_write_floats(
  sz_context_t *ctx,
  uint32_t name,
  float *values,
  size_t length
  );


// Reads a single float and writes it to `out`.
SZ_EXPORT
sz_response_t
sz_read_float(sz_context_t *ctx, uint32_t name, float *out);

// Reads an array of floats and returns it via `out`.
// The memory must be freed by the caller.
SZ_EXPORT
sz_response_t
sz_read_floats(
  sz_context_t *ctx,
  uint32_t name,
  float **out,
  size_t *length,
  sz_allocator_t *buf_alloc
  );


SZ_EXPORT
sz_response_t
sz_write_int(sz_context_t *ctx, uint32_t name, int32_t value);

SZ_EXPORT
sz_response_t
sz_write_ints(
  sz_context_t *ctx,
  uint32_t name,
  int32_t *values,
  size_t length
  );


// Reads a single int32_t and writes it to `out`.
SZ_EXPORT
sz_response_t
sz_read_int(sz_context_t *ctx, uint32_t name, int32_t *out);

// Reads an array of int32_t values and returns it via `out`.
// The memory must be freed by the caller.
SZ_EXPORT
sz_response_t
sz_read_ints(
  sz_context_t *ctx,
  uint32_t name,
  int32_t **out,
  size_t *length,
  sz_allocator_t *buf_alloc
  );


SZ_EXPORT
sz_response_t
sz_write_unsigned_int(sz_context_t *ctx, uint32_t name, uint32_t value);

SZ_EXPORT
sz_response_t
sz_write_unsigned_ints(
  sz_context_t *ctx,
  uint32_t name,
  uint32_t *values,
  size_t length
  );


// Reads a single uint32_t and writes it to `out`.
SZ_EXPORT
sz_response_t
sz_read_unsigned_int(sz_context_t *ctx, uint32_t name, uint32_t *out);

// Reads an array of uint32_t values and returns it via `out`.
// The memory must be freed by the caller.
SZ_EXPORT
sz_response_t
sz_read_unsigned_ints(
  sz_context_t *ctx,
  uint32_t name,
  uint32_t **out,
  size_t *length,
  sz_allocator_t *buf_alloc
  );


SZ_DEF_END


#endif /* end __SZ_SNOWBALL_H__ include guard */
