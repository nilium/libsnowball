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


/*! @file */


/*!
  @mainpage

  libsnowball is a simple serialization library written in C++ with a C API.
  It's intended to solve my ever-present problem of trying to avoid certain
  software licenses for certain projects and avoid dragging in multiple other
  libraries due to some other serialization libraries' requirements. Names will
  not be named because I probably forgot the names when I wrote the original
  snowball code years ago (at the time just called "serialize").

  To start, you'll want to check out the following sections:

  - @ref contexts
  - @ref streams
  - @ref allocators

  And probably in that order, though you can ignore allocators to start for the
  most part. Streams you'll need to learn eventually, but you can also start
  by just knowing about sz_stream_fopen() and sz_stream_close().


  ## License

  libsnowball is licensed under the MIT license:

  @verbinclude ./COPYING
*/


#ifndef __SZ_SNOWBALL_H__
#define __SZ_SNOWBALL_H__


//! @cond PREP

#if defined(__cplusplus)
# define SZ_DEF_BEGIN extern "C" {
# define SZ_DEF_END \
  }
  // Place the } on a second line because Sublime hates highlighting this.
# if __cplusplus >= 201103L
#   define SZ_TYPE_ENUM(TYPE) : TYPE
# else
#   define SZ_TYPE_ENUM(TYPE)
# endif
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


#if defined(_WIN32) || defined(__MINGW32__)
# if defined(SZ_BUILDING)
    // EXPORT
#   if defined(__GNUC__)
#     define SZ_EXPORT \
        __attribute__((dllexport))
#   else
#     define SZ_EXPORT \
        __declspec(dllexport)
#   endif
# else
    // IMPORT
#   if defined(__GNUC__)
#     define SZ_EXPORT \
        __attribute__((dllimport))
#   else
#     define SZ_EXPORT \
        __declspec(dllimport)
#   endif
# endif
# define SZ_HIDDEN
#else // def(win32) || def(mingw32)
  // the sane way
# define SZ_EXPORT \
    __attribute__((visibility("default")))
# define SZ_HIDDEN \
    __attribute__((visibility("hidden")))
#endif // !(win32 || mingw)


#pragma GCC diagnostic pop

//! @endcond

// Version numbers
//! @brief Major version number of libsnowball.
#define SZ_VERSION_MAJOR      1
//! @brief Minor version number of libsnowball.
#define SZ_VERSION_MINOR      2
//! @brief Reivsion number of libsnowball.
#define SZ_VERSION_REVISION   0


SZ_DEF_BEGIN


#define SZ_BIG_ENDIAN    1
#define SZ_LITTLE_ENDIAN 0
// The endianness used by snowball files. By default, uses little endian,
// because that's the usual endianness client-side.
#ifndef SZ_BASE_ENDIANNESS
# define SZ_BASE_ENDIANNESS SZ_LITTLE_ENDIAN
#endif
// mixed/pdp endianness not supported and probably never will be

// Host endianness.
// Little endian by default, as big endian isn't used anywhere I use snowball.
// It might be worthwhile to later replace this with something inserted at
// compile time or a configuration stage.
#ifndef SZ_ENDIANNESS
# define SZ_ENDIANNESS SZ_LITTLE_ENDIAN
#endif

#if SZ_ENDIANNESS != SZ_BASE_ENDIANNESS

SZ_EXPORT
uint32_t
sz_ntohl(uint32_t i);

SZ_EXPORT
uint16_t
sz_ntohs(uint16_t i);

# define sz_htonl(i) sz_ntohl((i))
# define sz_htons(i) sz_ntohs((i))

#else
// Base endianness is host endianness
# define sz_ntohl(i) (i)
# define sz_ntohs(i) (i)
# define sz_htonl(i) (i)
# define sz_htons(i) (i)
#endif


//! @brief Magic number type. Intended for use as a uint32_t.
typedef enum e_sz_magic SZ_TYPE_ENUM(uint32_t)
{
  /*!
    @brief Magic number at the head of every serialized file.

    Magic number placed at the head of every serialized stream as part of the
    file root. First two bytes are always SZ, last two bytes are an ASCII
    format version number.
  */
  SZ_MAGIC = 0x32305A53
} sz_magic_t;


/// @internal
/// @brief Returns the decimal form of a number character.
/// @endinternal
#define SZ_MAGIC_VER_SUB_ONE(m) \
  (((m) & 0xFF) - 0x30)

/*!
  @brief Macro used to get the format version of a serialized file's magic number.
*/
#define SZ_MAGIC_VER_INT(m) \
  (SZ_MAGIC_VER_SUB_ONE((m) >> 24) + SZ_MAGIC_VER_SUB_ONE((m) >> 16) * 10)


/*!
  @brief Chunk types.

  Chunk types used by libsnowball to mark what type a chunk is so as to prevent
  accidentally reading the wrong type from a serialized object.
*/
typedef enum e_sz_chunk_id SZ_TYPE_ENUM(uint32_t)
{
  //! @brief Invalid chunk type. Should never occur.
  SZ_INVALID_CHUNK = 0,
  //! @brief A compound chunk -- currently unused.
  SZ_COMPOUND_CHUNK = 1,
  //! @brief Compound reference. Chunk is an integer index into the compounds array.
  SZ_COMPOUND_REF_CHUNK = 2,
  //! @brief 32-bit float chunk.
  SZ_FLOAT_CHUNK = 3,
  //! @brief Unsigned 32-bit int chunk.
  SZ_UINT32_CHUNK = 4,
  //! @brief Signed 32-bit int chunk.
  SZ_SINT32_CHUNK = 5,
  //! @brief Array chunk (can contain any other chunk type).
  SZ_ARRAY_CHUNK = 6,
  //! @brief Bytes chunk -- arbitrary sequence of bytes.
  SZ_BYTES_CHUNK = 7,
  /*!
    @brief Null pointer chunk (used for null arrays/compounds and empty arrays).

    The null pointer chunk may substitute any compound, array, or bytes chunk
    whose pointer is NULL or length is 0.
  */
  SZ_NULL_POINTER_CHUNK = 8,
  //! @brief 64-bit float chunk -- currently unused.
  SZ_DOUBLE_CHUNK = 9,
  //! @brief Main data chunk.
  SZ_DATA_CHUNK = 10,
} sz_chunk_id_t;


//! @brief Responses from sz_ functions.
typedef enum e_sz_response SZ_TYPE_ENUM(int)
{
  //! @brief The function successfully completed.
  SZ_SUCCESS = 0,
  //! @brief Success. No error occurred.
  SZ_ERROR_NONE = SZ_SUCCESS,
  /*!
    Response when attempting to perform an operation on a context that cannot
    be open (e.g., setting the stream).
  */
  SZ_ERROR_CONTEXT_OPEN,
  /*!
    Response when attempting to perform an operation on a context that cannot
    be closed (e.g., reading is writing).
  */
  SZ_ERROR_CONTEXT_CLOSED,
  //! @brief Root is invalid, typically meaning it's not actually a serializable stream.
  SZ_INVALID_ROOT,
  //! @brief The first two bytes of the magic int are invalid.
  SZ_ERROR_MALFORMED_MAGIC_HEAD,
  //! @brief The version component of the magic int is invalid (too old or new).
  SZ_ERROR_MALFORMED_MAGIC_VERSION,
  /*!
    @brief Error from attempting to read an empty array.

    Error when attempting to read an empty array (length == 0). These don't
    occur in snowballs, as they're stored as a null chunk -- in other words,
    any zero-length array in a snowball means something went wrong.
  */
  SZ_ERROR_EMPTY_ARRAY,
  /*!
    @brief Error from providing a NULL pointer to something that doesn't like
           NULL pointers.

    A pointer is NULL.  For input operations, this means a required input
    (e.g., an array of floats) is NULL.  For output operations, all pointers
    other than the context are NULL when it's not permitted.  This may also
    mean a required argument to a set_* function is NULL.
  */
  SZ_ERROR_NULL_POINTER,
  //! @brief Exactly what it says on the tin.
  SZ_ERROR_NULL_CONTEXT,
  /*!
    The operation attempted cannot be performed.  This usually means
    attempting to perform a read operation on a writable serializer.
  */
  SZ_ERROR_INVALID_OPERATION,
  //! @brief The name being read is not the next name in the sequence
  SZ_ERROR_BAD_NAME,
  //! @brief What it says on the tin.  Again.
  SZ_ERROR_OUT_OF_MEMORY,
  //! @brief Attempted to read the wrong kind of data from the serializer.
  SZ_ERROR_WRONG_KIND,
  //! @brief Stream is somehow invalid
  SZ_ERROR_INVALID_STREAM,
  //! @brief Could not or cannot read from/write to the stream.
  SZ_ERROR_CANNOT_READ,
  //! @brief Could not or cannot write to a stream.
  SZ_ERROR_CANNOT_WRITE,
  //! @brief Reached EOF prematurely
  SZ_ERROR_EOF
} sz_response_t;


/*!
  @brief Context and stream modes.

  Context and stream modes provided when creating a context or initializing
  some streams (some streams may be read-write, in which case there's no point
  in distinguishing the mode).
*/
typedef enum e_sz_mode
{
  //! @brief Specifies a read-only stream or context.
  SZ_READER,
  //! @brief Specifies a write-only stream or context.
  SZ_WRITER
} sz_mode_t;


/*!
  @brief Opaque context data structure.

  A context is either read- or write-only and must always have an associated
  stream. Contexts themselves are opaque data structures, which is partly
  because it would be a bad idea to mess with their insides and partly because
  they're written using C++ and this API is designed for C.

  @ingroup contexts
*/
typedef struct s_sz_context sz_context_t;


//! @brief libsnowball version description returned by sz_version().
typedef struct s_sz_version
{
  //! @brief Major version number.
  int major;
  //! @brief Minor version number.
  int minor;
  //! @brief Revision number.
  int revision;
} sz_version_t;

/*!
  @brief Returns the library's version.

  Returns the library's version as an sz_version_t.

  @return
    An sz_version_t containing the library's version.
*/
SZ_EXPORT
sz_version_t
sz_version();


/*!
  @name Compound Callbacks
  @ingroup contexts

  ## Compounds

  Compound objects are simple, but can be a bit of a headache to understand at
  first. To put it simply, a compound object is a chunk that is parent to a
  sequence of other chunks -- which may include other compound chunks.

  Compound chunks themselves are only references, so they're never technically
  nested. Instead, libsnowball keeps a table of compounds and only writes their
  contents once. Every time a compound is written, therefore, two things happen:

  1. The compound is added to the compounds table and given an index into it.

  2. That compound's index is then written as an SZ_COMPOUND_REF_CHUNK wherever
    it was originally written. The compound itself is never placed directly in
    line with primitive data. This is to avoid problems that arise when nesting
    compound chunks (and why the SZ_COMPOUND_CHUNK type is no longer used).

  So, again, compounds are simply collections of other chunks that are written
  once, based off a pointer, and read once.

  The benefit of compounds is the read/written once part: if you find yourself
  repeatedly using the same data over and over, consider making it a compound.
  You can then very easily save a lot of calls to deserialize the same thing
  many times.


  ## Compound Callbacks

  Compound ops are read and written by two callback functions, both of which
  take the context being read from and an additional opaque pointer that may
  pointer to data needed by the context object -- the latter is user-provided
  and is neither required nor provided by libsnowball itself.

  A quick example of read and write compound functions follows:

  @code{.c}
  // Assume we'll never need more than 16 vectors
  static int vector_partition = 0;
  static struct { float v[3]; } vectors[16];

  static void read_vector(void **cmp, sz_context_t *ctx, void *rd) {
    (void)rd; // we're not using the reader data
    int index = vector_partition++;
    *cmp = &vectors[index];
    sz_read_float(&vectors[index].v[0], ctx, 'x');
    sz_read_float(&vectors[index].v[1], ctx, 'y');
    sz_read_float(&vectors[index].v[2], ctx, 'z');
  }

  static void write_vector(void *cmp, sz_context_t *ctx, void *wd) {
    (void)wd; // or the writer data
    float const *vec0 = &vectors[0];
    sz_write_float(vec0[0], ctx, 'x');
    sz_write_float(vec0[1], ctx, 'y');
    sz_write_float(vec0[2], ctx, 'z');
  }
  @endcode

  Given the above code, you can then call sz_write_compound() or
  sz_write_compounds() and pass write_vector to write an arbitrary vector from
  the array. The same goes for read_vector with sz_read_compound() and
  sz_read_compounds(). It's not necessary for compound memory to be dynamically
  allocated, unlike arrays in libsnowball (this may change).
*/
//! @{

/*!
  @brief Function pointer for writing compound objects.

  Function pointer for writing specific compounds. When writing a compound, you
  must pass a function that, given a compound, can write it to the context
  using sz_ functions (e.g., sz_write_unsigned_int() and so on). Compounds may
  be nested or self-referential.

  Receives an opaque writer_ctx pointer that is passed through
  sz_write_compound() / sz_write_compounds() to the writer function.

  @ingroup contexts
*/
typedef void (sz_compound_writer_fn_t)(
  void *compound,
  sz_context_t *ctx,
  void *writer_ctx
  );


/*!
  @brief Function pointer for reading compound objects.

  Reader function for compounds. When passed to sz_read_compound() (or the array
  equivalent), this function allocates memory for a compound and reads the
  compound's data from the context.

  Once memory has been allocated for the compound, it's important that the
  compound pointer be written to the `void **compound` output. Otherwise, if
  further compounds are read while a compound is being read, they will be given
  a NULL pointer, as the compound currently being read will have been marked as
  unpacked and no longer passed to a reader function.

  The memory for the compound allocated by the reader must be freed manually.
  The context does not own it.

  @remarks An important point on how sz_compound_reader_fn_t should work:

  @remarks
    When the reader is called, the output pointer, compound, is only guaranteed
    to be valid before any other compound reader calls are made to the
    serializer. Second, the only way to deal with semi-circular references is
    to allocate, write the compound pointer out, and continue reading the
    contents of the compound only afterward. Otherwise, any compounds that
    reference the compound being read will receive a NULL pointer and further
    attempts will not be made to unpack the compound.

  @remarks
    So, an example function might look like:

  @remarks
    @code{.c}
    sz_entity_reader(void **p, sz_context_t *ctx, void *rd_ctx)
    {
      char *name;
      entity_t *entity = entity_new(..);  // Allocate
      *p = entity;                        // Write to p
      // Do whatever else you need to, never access p again
      sz_read_bytes(&name, NULL, ctx, KEY_ENTITY_NAME, NULL);
      ...
    }
    @endcode

  @ingroup contexts
*/
typedef void (sz_compound_reader_fn_t)(
  void **compound,
  sz_context_t *ctx,
  void *reader_ctx
  );

//! @}


typedef struct s_sz_allocator sz_allocator_t;

/*!
  @brief Basic allocator ops structure for malloc and free.

  Basic allocator structure. Only requires malloc and free implementations.
  Allocator functions are passed a pointer to the containing allocator in the
  event that they need to pull further data from the allocator.

  @ingroup allocators
*/
struct s_sz_allocator
{
  //! @brief malloc function used by an allocator.
  void *(*malloc)(size_t sz, sz_allocator_t *allocator);
  //! @brief free function used by an allocator.
  void (*free)(void *ptr, sz_allocator_t *allocator);
};


typedef struct s_sz_stream sz_stream_t;

/*!
  @defgroup streams Streams
  @brief Streams and stream functions.

  Streams are a simple collection of stream functions that provide read, write,
  seek, eof, and close functions to provide a basic abstract stream object.

  For a simple example of stream functions, see src/fstream.cc in the
  libsnowball source code.
*/
//! @{

/*!
  @brief Basic stream ops callbacks.

  An sz_stream_t defines a collection of stream functions that operate on some
  arbitrary type of stream. It's intended to provide a bare minimum of
  functionality so as to avoid getting bogged down with things like formatting
  and so on.

  As a result, only five operations are supported:

  - Read
  - Write
  - Seek
  - Check for EOF
  - Close

  The last is special in that it requires both that the stream be closed and
  all memory associated with the stream be freed (including the stream itself).
  There is no tell operation, as seek must return the resulting offset into the
  stream, therefore making any seek to an offset of 0 from the current position
  the same as a tell. Streams should optimize for that case when used for
  reading.

  It is possible that a stream will never return true for EOF, and at no point
  does a context depend on it returning true. It is only necessary for avoiding
  reads beyond the bounds of a stream.
*/
struct s_sz_stream
{
  /*!
    @brief Reads length bytes from a stream.

    Reads length bytes from a stream into out and returns the number of bytes
    actually read. Returning a number less than length is treated as an error,
    though typically it just means EOF was reached before it was expected, in
    which case the snowball may be malformed.
  */
  size_t (*read)(void *out, size_t length, sz_stream_t *stream);

  /*!
    @brief Writes length bytes to a stream.

    Writes length bytes from in to a stream, returns the number of bytes
    actually written. If anything other than length is returned, snowball
    assumes an error occurred. length 0 should be a no-op and ignored.
  */
  size_t (*write)(const void *in, size_t length, sz_stream_t *stream);

  /*!
    @brief Seeks to an offset in a stream from a point.

    Seeks to the given offset (only SEEK_CUR and SEEK_SET need to be
    supported). Returns the resulting offset into the stream.
  */
  off_t (*seek)(off_t off, int whence, sz_stream_t *stream);

  /*!
    @brief Returns whether a stream is at its EOF.

    It is not necessary for a stream to ever return true or check for this,
    provided that there is no chance of a stream ever reading or writing past
    its EOF. This function is only necessary for checking once a read or write
    has failed.
  */
  int (*eof)(sz_stream_t *stream);

  /*!
    @brief Closes and deallocates a stream.

    Disposes of a stream and deallocates it. Streams are considered invalid
    after this and are free to crash or explode or do whatever they want if
    accessed again.
  */
  void (*close)(sz_stream_t *stream);
};



/*!
  @brief Opens a file stream.

  Opens an sz_stream_t for the given filename either as read- or write-only and
  returns it.

  If alloc is null, the function uses the default allocator.

  @param filename
    The path of the file to open.
  @param mode
    The mode to open in: either read or write.
  @param alloc
    The allocator to use when allocating the stream object.
  @return
    A stream object for a file.
*/
SZ_EXPORT
sz_stream_t *
sz_stream_fopen(const char *filename, sz_mode_t mode, sz_allocator_t *alloc);

/*!
  @brief Returns a stream that is functionally equivalent to `/dev/null`.

  Returns a stream that does nothing and always fails to read/write/seek.
  Handy for testing, occasionally.

  @note
    The same pointer is always returned by sz_stream_null(), so it's imperative
    that you do not modify the returned stream.

  @return
    A stream that's the functional equivalent of reading from/writing to
  `/dev/null`.
*/
SZ_EXPORT
sz_stream_t *
sz_stream_null();

/*!
  @brief Reads length bytes a stream to an out buffer.

  Reads length bytes from a stream to an out buffer, returning the number of
  bytes actually read.

  @param out
    Where to read bytes to.
  @param length
    The number of bytes to read.
  @param stream
    A stream to read from.
  @return
    The number of bytes read.
*/
SZ_EXPORT
size_t
sz_stream_read(void *out, size_t length, sz_stream_t *stream);

/*!
  @brief Writes length bytes from an in buffer to a stream.

  Writes length bytes from an input buffer to a stream, returning the number of
  bytes written.

  @param in
    Where to read bytes from.
  @param length
    The number of bytes to write.
  @param stream
    A stream to write to.
  @return
    The number of bytes written.
*/
SZ_EXPORT
size_t
sz_stream_write(const void *in, size_t length, sz_stream_t *stream);

/*!
  @brief Seeks from a point to an offset in a stream.

  Seeks from a point given by whence by an offset. Only SEEK_CUR and SEEK_SET
  are required to be supported -- SEEK_END should not be used.

  @param off
    The offset to seek by.
  @param whence
    Where the offset is from.
  @param stream
    A stream to seek in.
  @return
    The offset into the stream, from the beginning, after the seek.
*/
SZ_EXPORT
off_t
sz_stream_seek(off_t off, int whence, sz_stream_t *stream);

/*!
  @brief Same as `sz_stream_seek(0, SEEK_CUR, stream).`

  Gets the current position of a stream using sz_stream_seek().

  @param stream
    A stream to get the position of.
  @return
    The position returned by `sz_stream_seek(0, SEEK_CUR, stream)`.
*/
SZ_EXPORT
off_t
sz_stream_tell(sz_stream_t *stream);

/*!
  @brief Returns whether a stream is at its EOF (or something equivalent).

  Returns whether the stream is at its EOF or has reached something equivalent
  to an EOF for it. Streams are not required to check for this, as it is only
  used for error handling when a read or write fails, allowing a specific
  error for reaching an EOF while reading or writing.

  @param stream
    A stream to check.
  @return Non-zero if the stream is at EOF, otherwise zero.
*/
SZ_EXPORT
int
sz_stream_eof(sz_stream_t *stream);

/*!
  @brief Closes a stream and deallocates any memory in use by it.

  Closes a stream. This does three things: unwritten buffered data in the
  stream is flushed, the stream is closed, and all resources associated with
  the stream are released including the stream itself. So, for example,
  with a file stream, this closes the file and deallocates the stream.

  For some streams, this is a nop (e.g., the stream returned by
  sz_stream_null() does nothing when closing, as the same stream is returned
  every time).

  @param stream
    A stream to close.
*/
SZ_EXPORT
void
sz_stream_close(sz_stream_t *stream);

//! @}



/////////// Allocation

/*!
  @defgroup allocators Allocators

  @brief Handling memory allocation and deallocation.

  Allocators are simple structures that hold two memory allocation/deallocation
  function: malloc and free. These may be implemented however you see fit, so
  long as they do what malloc and free are expected to do.

  The default allocator provided by libsnowball uses malloc and free, so it's
  usually good enough. If you've compiled libsnowball as a shared library,
  however, I strongly recommend you replace the default allocator (using
  sz_set_default_allocator()) with one that uses the host process's memory
  allocation functions, even if they're still malloc/free, since it's not a
  great idea to leave the memory in the ownership of a shared library.
*/
//! @{

/*!
  @brief Returns a default allocator -- by default, it uses the C stdlib's
         malloc and free functions.

  Returns a default allocator. The default-default allocator -- that is, the
  allocator prior to calls to sz_set_default_allocator(), uses the C stdlib's
  malloc and free functions. In most cases, this is sufficient, but keeping the
  default is discouraged if libsnowball is a shared library (since this will
  place memory allocated by it in the ownership of the shared library, which
  isn't always what you want).

  @returns The current default allocator.

  @ingroup allocators
*/
SZ_EXPORT
sz_allocator_t *
sz_default_allocator();

/*!
  @brief Sets the default allocator.

  Sets the default allocator to alloc. If alloc is NULL, it reverts to the
  library's default allocator (aka malloc and free). This can be useful if you
  want to ensure memory is allocated using a specific allocator by default
  rather than using malloc/free as provided by the library.

  @param alloc
    The allocator to make the new default. If NULL, this reverts the default
    allocator back to the library's default, rather than actually setting it
    to NULL, because that would be insane.
*/
SZ_EXPORT
void
sz_set_default_allocator(sz_allocator_t *alloc);

/*!
  @brief Sets the malloc and free functions used by the library's default
  allocator.

  Sets the library's default allocator functions to malloc and free
  implementations. If either function pointer is NULL, those function pointers
  are ignored and reset to the default library malloc/free.

  @param malloc_
    The malloc function to use. If NULL, uses the library's malloc function.
  @param free_
      The free function to use. If NULL, uses the library's free function.
*/
SZ_EXPORT
void
sz_set_default_malloc_free(void *(*malloc_)(size_t), void (*free_)(void *));

/*!
  @brief Frees previously allocated memory.

  Frees the given pointer using the allocator. The allocator must be the same
  used to allocate the pointer in the first place, otherwise the allocator's
  behavior is undefined.

  @param ptr
    A pointer to memory previously allocated by the allocator.
  @param allocator
    An allocator used to free the memory.
*/
SZ_EXPORT
void
sz_free(void *ptr, sz_allocator_t *allocator);

/*!
  @brief Allocates memory.

  Allocates size bytes using an allocator and returns it. Allocators are only
  required to provide at least size bytes -- they may allocate more if desired.

  @note The memory is not necessarily required to be aligned but it's kind of
  assumed it will be since otherwise you're probably looking at a world of pain.
  Choose your allocators wisely. If in doubt, use the default allocator, since
  it's just using malloc/free and those have probably been optimized enough for
  general use.

  @param size
    The size in bytes of the memory to allocate.
  @param allocator
    An allocator to allocate the requested memory with.
  @return
    A pointer to the memory allocated if successful, otherwise NULL.
*/
SZ_EXPORT
void *
sz_malloc(size_t size, sz_allocator_t *allocator);

//! @}



/*!
  @defgroup contexts Contexts

  @brief The serializers and deserializers of libsnowball.

  Contexts are used by libsnowball to collect read/write operations and their
  data in one place. All contexts are created using sz_new_context(), and may
  be either readers (SZ_READER) or writers (SZ_WRITER). Once a context has
  been created, it's necessary to set its stream using sz_set_stream() and then
  open it using sz_open().

  Once open, a context may be used for either `sz_read_` or `sz_write_` calls.
*/

/*!
  @brief Creates a new context.

  Creates a new context with the given mode and allocator and returns it. The
  returned context must have its stream set before it can be opened for reading
  or writing.

  @param mode
    SZ_READER or SZ_WRITER. The former to deserialize, the latter to serialize.
  @param allocator
    The allocator to use when allocating the context and its memory.
    If NULL, uses the default allocator.
  @return
    A new context pointer on success, or NULL on failure.

  @ingroup contexts
*/
SZ_EXPORT
sz_context_t *
sz_new_context(sz_mode_t mode, sz_allocator_t *allocator);

/*!
  @brief Destroys a context.

  Destroys a context.

  @remarks It is permitted to destroy contexts that are open, provided they
  aren't performing any operations at the time (in other words, do not call
  this from within compound reader/writer, allocator, or stream functions,
  more or less).

  @param ctx
    The context to destroy.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_destroy_context(sz_context_t *ctx);


/*!
  @brief Opens a context for de/serialization.

  Opens the context for reading or writing to/from a stream.

  @param ctx
    A context to open.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_open(sz_context_t *ctx);

/*!
  @brief Closes a context and finishes de/serialization.

  Ends de/serialization and closes the context.
  If serializing, the serialized data will be written to the provided stream.

  @param ctx
    A context to close.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_close(sz_context_t *ctx);

/////////// Attributes (use before sz_open)

// Input / output file
/*!
  @brief Sets a context's stream object.

  Sets the context's stream. This must be called before opening a context and
  may not be called again until the context has been closed.

  @param ctx
    A context to set the stream for.
  @param stream
    A stream to give the context.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_set_stream(sz_context_t *ctx, sz_stream_t *stream);

/*!
  @brief Get an error string describing the most recent error in a context.

  Returns a NULL-terminated error string describing the most recent error in a
  context. This pointer is managed by libsnowball and should not be freed or
  modified.

  @param ctx
    The context to get an error string from.
  @return
    NULL on error or with no error, otherwise an error string.

  @ingroup contexts
*/
SZ_EXPORT
const char *
sz_get_error(sz_context_t *ctx);

//////////// Read/write operations


/*! @name Write Ops */
//! @{

/*!
  @brief Writes a compound to a context.

  Writes a compound object to a writer context with the given name using a
  compound writer function.

  @param ctx
    A context to write to.
  @param name
    The name of the chunk to write.
  @param compound_in
    A compound pointer to write.
  @param writer
    The compound writer function. May not be NULL.
  @param writer_ctx
    Opaque pointer passed to the compound writer. May be NULL.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_write_compound(
  void *compound_in,
  sz_context_t *ctx,
  uint32_t name,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx);

/*!
  @brief Writes an array of compounds to a context.

  Writes an array of compound objects to a writer context with the given name
  using a compound writer function.

  @param ctx
    A context to write to.
  @param name
    The name of the chunk to write.
  @param compounds_in
    An array of compounds to write.
  @param length
    The number of compounds to write.
  @param writer
    The compound writer function. May not be NULL.
  @param writer_ctx
    Opaque pointer passed to the compound writer. May be NULL.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_write_compounds(
  void **compounds_in,
  size_t length,
  sz_context_t *ctx,
  uint32_t name,
  sz_compound_writer_fn_t *writer,
  void *writer_ctx
  );

/*!
  @brief Writes an array of bytes to a context.

  Writes an array of bytes with the given length to a context.

  @param ctx
    A context to write to.
  @param name
    The name of the chunk to write.
  @param values
    An array of bytes to write.
  @param length
    The number of bytes to write.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_write_bytes(
  const void *values,
  size_t length,
  sz_context_t *ctx,
  uint32_t name
  );

/*!
  @brief Writes a float to a context.

  Writes a single float to the context with the given name.

  @param value
    The float to write.
  @param ctx
    A context to read from.
  @param name
    The name of the chunk to write.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_write_float(float value, sz_context_t *ctx, uint32_t name);

/*!
  @brief Writes an array of floats to a context.

  Writes an array of floats to the context with the given name.

  @param values
    An array of floats to write.
  @param length
    The number of floats to write.
  @param ctx
    A context to write to.
  @param name
    The name of the chunk to write.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_write_floats(
  float *values,
  size_t length,
  sz_context_t *ctx,
  uint32_t name
  );

/*!
  @brief Writes an int to a context.

  Writes a single int to the context with the given name.

  @param value
    The int to write.
  @param ctx
    A context to read from.
  @param name
    The name of the chunk to write.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_write_int(int32_t value, sz_context_t *ctx, uint32_t name);

/*!
  @brief Writes an array of 32-bit signed ints to a context.

  Writes an array of 32-bit signed ints to the context with the given name.

  @param values
    An array of ints to write.
  @param length
    The number of ints to write.
  @param ctx
    A context to write to.
  @param name
    The name of the chunk to write.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/

SZ_EXPORT
sz_response_t
sz_write_ints(
  int32_t *values,
  size_t length,
  sz_context_t *ctx,
  uint32_t name
  );

/*!
  @brief Writes an unsigned int to a context.

  Writes a single unsigned int to the context with the given name.

  @param value
    The unsigned int to write.
  @param ctx
    A context to read from.
  @param name
    The name of the chunk to write.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_write_unsigned_int(uint32_t value, sz_context_t *ctx, uint32_t name);

/*!
  @brief Writes an array of 32-bit unsigned ints to a context.

  Writes an array of 32-bit unsigned ints to the context with the given name.

  @param values
    An array of unsigned ints to write.
  @param length
    The number of unsigned ints to write.
  @param ctx
    A context to write to.
  @param name
    The name of the chunk to write.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_write_unsigned_ints(
  uint32_t *values,
  size_t length,
  sz_context_t *ctx,
  uint32_t name
  );

//! @}


/*! @name Read Ops */
//! @{

/*!
  @brief Reads a compound from a context.

  @brief Reads a serialized compound object.

  Reads a compound object from a reader context with the given name using a
  compound reader function.

  @param compound_out
    A pointer to write the compound pointer to. There are more confusing ways
    to word this parameter's description.
  @param ctx
    A context to read from.
  @param name
    The name of the chunk to read.
  @param reader
    The function used to read the compound. May not be null. If the compound
    was previously deserialized, this function will not be called.
  @param reader_ctx
    Opaque data pointer passed to the reader function as context. May be null.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_compound(
  void **compound_out,
  sz_context_t *ctx,
  uint32_t name,
  sz_compound_reader_fn_t *reader,
  void *reader_ctx
  );

/*!
  @brief Reads an array of compounds from a context.

  Reads an array of compound objects with the given name using a compound
  reader function. The returned array of objects must be freed by the caller
  using the allocator provided by the caller. The returned array is an array
  of pointers, so your output argument is a pointer to an array of pointer.
  I only feel the need to mention this because `void ***` is a scary type
  for obvious reasons.

  @param compounds_out
    A pointer to a pointer that will receive the array. May be null, in which
    case no array is allocated and the chunk is effectively skipped
    if it matches. If *compounds_out is null, a buffer is allocated using the
    provided allocator and returned via *compounds_out. Otherwise, it is
    assumed that you have passed a pre-allocated buffer of the correct or
    greater size and want the compound pointers written to it.
  @param length
    A pointer to a size_t that will receive the length of the array.
    May be null.
  @param ctx
    The context to read from.
  @param name
    The name of the chunk to read. Must be the next chunk name in the context.
  @param reader
    The function used to read the compounds in the array. May not be null. If
    a compound was previously deserialized, this function will not be called
    for that compound.
  @param reader_ctx
    Opaque data pointer passed to the reader function as context. May be null.
  @param buf_alloc
    The allocator to use to allocate the array. If null, uses the default
    allocator.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_compounds(
  void ***compounds_out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_compound_reader_fn_t *reader,
  void *reader_ctx,
  sz_allocator_t *buf_alloc
  );

/*!
  @brief Reads an array of bytes from a context.

  Reads an array of bytes from a context and returns it via `out`.
  The memory must be freed by the caller using the allocator provided.

  @param out
    A pointer to a pointer that will receive the array. May be null, in which
    case no array is allocated and the chunk is effectively skipped
    if it matches. If *out is non-null, it's assumed the block receiving the
    values is preallocated and can hold at least as many bytes as held by the
    chunk.
  @param length
    A pointer to a size_t that will receive the length of the array.
    May be null.
  @param ctx
    The context to read from.
  @param name
    The name of the chunk to read. Must be the next chunk name in the context.
  @param buf_alloc
    The allocator to use to allocate the array. If null, uses the default
    allocator.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_bytes(
  void **out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_allocator_t *buf_alloc
  );

/*!
  @brief Reads a float from a context.

  Reads a single float and writes it to `out`.

  @param out
    A pointer to a float to store the read value in. May be null.
  @param ctx
    A context to read from.
  @param name
    The name of the chunk to read.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_float(float *out, sz_context_t *ctx, uint32_t name);

/*!
  @brief Reads an array of floats from a context.

  Reads an array of floats and returns it via `out`.

  The array returned via `out` must be freed by the caller using the alloactor
  provided by the caller.

  @param out
    A pointer to a pointer that will receive the array. May be null, in which
    case no array is allocated and the chunk is effectively skipped
    if it matches. If *out is non-null, it's assumed the block receiving the
    values is preallocated and can hold at least as many values as held by the
    chunk.
  @param length
    A pointer to a size_t that will receive the length of the array.
    May be null.
  @param ctx
    The context to read from.
  @param name
    The name of the chunk to read. Must be the next chunk name in the context.
  @param buf_alloc
    The allocator to use to allocate the array. If null, uses the default
    allocator.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_floats(
  float **out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_allocator_t *buf_alloc
  );

/*!
  @brief Reads an int from a context.

  Reads a single int32_t and writes it to `out`.

  @param out
    A pointer to an int to store the read value in. May be null.
  @param ctx
    A context to read from.
  @param name
    The name of the chunk to read.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_int(int32_t *out, sz_context_t *ctx, uint32_t name);

/*!
  @brief Reads an array of int32_t values from a context.

  Reads an array of int32_t values and returns it via `out`.

  The array returned via `out` must be freed by the caller using the alloactor
  provided by the caller.

  @param out
    A pointer to a pointer that will receive the array. May be null, in which
    case no array is allocated and the chunk is effectively skipped
    if it matches. If *out is non-null, it's assumed the block receiving the
    values is preallocated and can hold at least as many values as held by the
    chunk.
  @param length
    A pointer to a size_t that will receive the length of the array.
    May be null.
  @param ctx
    The context to read from.
  @param name
    The name of the chunk to read. Must be the next chunk name in the context.
  @param buf_alloc
    The allocator to use to allocate the array. If null, uses the default
    allocator.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_ints(
  int32_t **out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_allocator_t *buf_alloc
  );

/*!
  @brief Reads an unsigned int from a context.

  Reads a single uint32_t and writes it to `out`.

  @param out
    A pointer to an unsigned int to store the read value in. May be null.
  @param ctx
    A context to read from.
  @param name
    The name of the chunk to read.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_unsigned_int(uint32_t *out, sz_context_t *ctx, uint32_t name);

/*!
  @brief Reads an array of uint32_t values from a context.

  Reads an array of uint32_t values and returns it via `out`.

  The array returned via `out` must be freed by the caller using the alloactor
  provided by the caller.

  @param out
    A pointer to a pointer that will receive the array. May be null, in which
    case no array is allocated and the chunk is effectively skipped
    if it matches. If *out is non-null, it's assumed the block receiving the
    values is preallocated and can hold at least as many values as held by the
    chunk.
  @param length
    A pointer to a size_t that will receive the length of the array.
    May be null.
  @param ctx
    The context to read from.
  @param name
    The name of the chunk to read. Must be the next chunk name in the context.
  @param buf_alloc
    The allocator to use to allocate the array. If null, uses the default
    allocator.
  @return
    A response code. SZ_SUCCESS on success, otherwise an error.

  @ingroup contexts
*/
SZ_EXPORT
sz_response_t
sz_read_unsigned_ints(
  uint32_t **out,
  size_t *length,
  sz_context_t *ctx,
  uint32_t name,
  sz_allocator_t *buf_alloc
  );

//! @}


SZ_DEF_END


#endif /* end __SZ_SNOWBALL_H__ include guard */
