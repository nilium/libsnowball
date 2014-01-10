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

#ifndef __READ_CONTEXT_HH__
#define __READ_CONTEXT_HH__


#include "context.hh"
#include "chunk.hh"
#include "bufstream.hh"
#include "allocator_wrapper.hh"

#include <map>
#include <vector>


struct SZ_HIDDEN sz_read_context_t : public s_sz_context
{
private:

  struct unpacked_compound_t {
    off_t offset;     // Position of the item in the input file
    void *value;      // may be NULL
    // whether the compound has been read -- if this is true, no further
    // attempts are made to read the compound and its unpacked value will be
    // used instead. This can help with cycles, though it has one drawback: the
    // compounds pointer's value must be set before the next compound in the
    // cycle is read, otherwise it will receive a null compound pointer.
    bool unpacked;
  };

  static const unpacked_compound_t default_unpacked_compound;

  typedef std::vector<off_t, sz_cxx_allocator_t<off_t>> offsets_t;
  typedef std::vector<
    unpacked_compound_t,
    sz_cxx_allocator_t<unpacked_compound_t>
    > compounds_t;

  compounds_t compounds;
  offsets_t offsets;

  // I can't track whether the context is open by whether something exists, so
  // just keep a flag I can set/unset...
  bool is_open;

public:

  virtual
  sz_mode_t
  mode() const;

  sz_read_context_t(sz_allocator_t *);

  virtual
  ~sz_read_context_t();


  // Roots
  sz_response_t
  read_root(sz_root_t *root);


  // Headers
  sz_response_t
  read_header(
    sz_header_t *header,
    sz_chunk_id_t type,
    uint32_t name,
    bool null_allowed
    );


  // Arrays
  sz_response_t
  read_array_header(
    sz_array_t *chunk,
    sz_chunk_id_t type,
    uint32_t name
    );

  sz_response_t
  read_array_body(
    void **buf_out,
    size_t *length,
    const sz_array_t *chunk,
    sz_allocator_t *alloc
    );


  // Info stack
  void
  push_stack();

  void
  pop_stack();


  // Compounds
  sz_response_t
  read_compound(
    void **compound,
    uint32_t name,
    sz_compound_reader_fn_t reader,
    void *reader_ctx
    );

  sz_response_t
  read_compound_array(
    void ***compound,
    size_t *length,
    uint32_t name,
    sz_compound_reader_fn_t reader,
    void *reader_ctx,
    sz_allocator_t *alloc
    );

  void *
  get_compound(
    uint32_t index,
    sz_compound_reader_fn_t reader,
    void *reader_ctx
    );


  // Primitives
  sz_response_t
  read_primitive(
    void *out,
    sz_chunk_id_t type,
    size_t type_size,
    uint32_t name
    );

  sz_response_t
  read_primitive_array(
    void **out,
    size_t *length,
    sz_chunk_id_t type,
    size_t type_size,
    uint32_t name,
    sz_allocator_t *buf_alloc
    );

  sz_response_t
  read_bytes(
    void **out,
    size_t *length,
    uint32_t name,
    sz_allocator_t *buf_alloc
    );

  // Reading
  sz_response_t
  begin_read();

  sz_response_t
  end_read();


  // Open / close ops
  virtual
  sz_response_t
  open();

  virtual
  sz_response_t
  close();

  virtual
  bool
  opened() const;

};


#endif /* end __READ_CONTEXT_HH__ include guard */
