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

#include <map>
#include <vector>


struct SZ_HIDDEN sz_read_context_t : public s_sz_context
{
private:

  struct unpacked_compound_t {
    void *value;      // NULL if not yet unpacked
    off_t position;   // Position of the item in the input file
  };

  typedef std::vector<off_t> offsets_t;
  typedef std::vector<unpacked_compound_t> compounds_t;

  compounds_t compounds;


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
    uint32_t name,
    sz_chunk_id_t type
    );


  // Arrays
  sz_response_t
  read_array_header(
    sz_array_t *chunk,
    uint32_t name,
    sz_chunk_id_t type
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


  // Primitives
  sz_response_t
  read_primitive(
    void *out,
    sz_chunk_id_t type,
    size_t type_size,
    uint32_t name
    );

  // Reading
  sz_response_t
  begin_read();

  sz_response_t
  end_read();


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


#endif /* end __READ_CONTEXT_HH__ include guard */
