
#ifndef __MEM_H__
#define __MEM_H__

#include <stdlib.h>

#define new(obj) ((obj *)mem_alloc(sizeof(obj)))
#define delete(ptr) (mem_free(ptr))

static inline void * mem_alloc(size_t sz)
{
	return malloc(sz);
}

static inline void * mem_free(void * ptr)
{
	free(ptr);
}

#endif
