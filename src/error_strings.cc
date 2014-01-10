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

#include "error_strings.hh"


const char *const sz_errstr_no_error =
  "No error.";

const char *const sz_errstr_null_context =
  "Null serializer context.";

const char *const sz_errstr_invalid_root =
  "Invalid magic number for root.";

const char *const sz_errstr_invalid_magic_head =
  "First two bytes of the magic number for the stream are invalid.";

const char *const sz_errstr_invalid_magic_version =
  "The version of this snowball is not supported.";

const char *const sz_errstr_wrong_kind =
  "Invalid chunk header: wrong chunk kind.";

const char *const sz_errstr_bad_name =
  "Invalid chunk header: wrong chunk name.";

const char *const sz_errstr_cannot_read =
  "Unable to read from stream.";

const char *const sz_errstr_cannot_write =
  "Unable to write to stream.";

const char *const sz_errstr_eof =
  "Unexpected end of stream reached.";

const char *const sz_errstr_write_on_read =
  "Cannot perform write operation on read-serializer.";

const char *const sz_errstr_read_on_write =
  "Cannot perform read operation on write-serializer.";

const char *const sz_errstr_compound_reader_null =
  "Failed to deserialize compound object with reader: reader returned NULL.";

const char *const sz_errstr_already_closed =
  "Cannot close serializer that isn't open.";

const char *const sz_errstr_already_open =
  "Invalid operation on open context.";

const char *const sz_errstr_open_set_stream =
  "Cannot set stream for open serializer.";

const char *const sz_errstr_null_stream =
  "Stream is NULL.";

const char *const sz_errstr_empty_array =
  "Array is empty.";

const char *const sz_errstr_nomem =
  "Allocation failed.";
