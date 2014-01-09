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

#ifndef __CHUNK_HH__
#define __CHUNK_HH__


typedef struct s_sz_root {
  // magic number -- should be SZ_MAGIC
  uint32_t magic;
  // size of the serializable data including this root
  uint32_t size;
  // offsets are from the root
  uint32_t num_compounds;
  // Always immediately follows the root, so = sizeof(root)
  uint32_t mappings_offset;
  // Follows mappings
  uint32_t compounds_offset;
  // Follows compounds
  uint32_t data_offset;

  // mappings
  // data
  // compounds
} sz_root_t;


typedef struct s_sz_header {
  uint32_t kind;  // chunk type
  uint32_t name;  // chunk name
  uint32_t size;  // length including this header
} sz_header_t;


typedef struct s_sz_compound_ref {
  sz_header_t base;
  uint32_t index;
} sz_compound_ref_t;


typedef struct s_sz_array {
  sz_header_t base;
  uint32_t length;
  uint32_t type;
} sz_array_t;


#endif /* end __CHUNK_HH__ include guard */
