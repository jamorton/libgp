
#ifndef GP_INCLUDE_COMMON_H
#define GP_INCLUDE_COMMON_H

#include <stdint.h>

#ifndef GP_MAX_REGISTERS
  #define GP_MAX_REGISTERS 5
#endif

#ifndef GP_TYPE
  #define GP_TYPE double
#endif

typedef unsigned int uint;
typedef unsigned long long ulong;

typedef GP_TYPE gp_num_t;
typedef double gp_fitness_t;

#define gp_min(a,b) ((a)<(b)?(a):(b))
#define gp_max(a,b) ((a)>(b)?(a):(b))

#define gp_likely(x)       __builtin_expect((x),1)
#define gp_unlikely(x)     __builtin_expect((x),0)

// Random number generation utilities
// ----------------------------------

#define SFMT_HAVE_SSE2 1
#define SFMT_MEXP 19937
#include "SFMT.h"

extern sfmt_t _sfmt;

static inline uint umin(uint a, uint b)
{
	return a < b ? a : b;
}

static inline uint urand(uint low, uint high)
{
	return sfmt_genrand_uint32(&_sfmt) % (high - low) + low;
}

static inline double rand_double(void)
{
	return sfmt_genrand_real1(&_sfmt);
}

static inline float rand_float(void)
{
	return (float)rand_double();
}

#if GP_TYPE==float
  static inline GP_TYPE rand_num(void) { return rand_float(); }
#elif GP_TYPE==double
  static inline GP_TYPE rand_num(void) { return rand_double(); }
#else
  #error Invalid type specified for GP_TYPE
#endif

#endif
