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

#ifndef __WRITE_CONTEXT_HH__
#define __WRITE_CONTEXT_HH__


#include "context.hh"
#include "chunk.hh"

#include <map>
#include <vector>

struct SZ_HIDDEN sz_write_context_t : public s_sz_context
{
private:
  typedef std::vector<sz_stream_t *> stream_stack_t;
  typedef std::map<void *, uint32_t> compound_map_t;

  stream_stack_t streams;
  stream_stack_t compound_streams;
  compound_map_t compound_indices;


public:

  virtual
  sz_mode_t
  mode() const;

  sz_write_context_t(sz_allocator_t *);

  virtual
  ~sz_write_context_t();


  // Roots
  sz_response_t
  write_root(const sz_root_t &root);


  // Headers
  sz_response_t
  write_header(const sz_header_t &header);


  // Compounds
  uint32_t
  new_compound(void *compound);

  uint32_t
  store_compound(
    void *compound,
    sz_compound_writer_fn_t *writer,
    void *writer_ctx
    );

  sz_response_t
  write_compound(
    void *compound,
    sz_compound_writer_fn_t *writer,
    void *writer_ctx,
    uint32_t name
    );

  sz_response_t
  write_compound_array(
    void **compounds,
    size_t length,
    sz_compound_writer_fn_t *writer,
    void *writer_ctx,
    uint32_t name
    );


  // Info stack
  void
  push_stack();

  void
  pop_stack();


  // Null
  sz_response_t
  write_null_pointer(uint32_t name);


  // Primitives
  sz_response_t
  write_primitive(
    const void *input,
    sz_chunk_id_t type,
    size_t type_size,
    uint32_t name
    );

  sz_response_t
  write_primitive_array(
    const void *input,
    sz_chunk_id_t type,
    size_t type_size,
    size_t length,
    uint32_t name
    );


  // Open / flush / close ops
  virtual
  sz_response_t
  open();

  virtual
  sz_response_t
  flush();

  virtual
  sz_response_t
  close();
};


#endif /* end __WRITE_CONTEXT_HH__ include guard */
