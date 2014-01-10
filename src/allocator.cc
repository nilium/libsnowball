#include <snowball.h>


SZ_DEF_BEGIN


void
sz_free(void *ptr, sz_allocator_t *alloc)
{
  alloc->free(ptr, alloc);
}


void *
sz_malloc(size_t size, sz_allocator_t *alloc)
{
  return alloc->malloc(size, alloc);
}


SZ_DEF_END
