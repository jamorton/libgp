
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

// Random number generation utilities
// ----------------------------------

#define HAVE_SSE2 1
#define MEXP 19937
#include "SFMT/SFMT.h"

static inline uint umin(uint a, uint b)
{
	return a < b ? a : b;
}

static inline uint urand(uint low, uint high)
{
	return gen_rand32() % (high - low) + low;
}

static inline double rand_double(void)
{
	// FIXME: generate a 64-bit number here?
	return (double)(gen_rand32()) / UINT32_MAX;
}

static inline float rand_float(void)
{
	return (float)(gen_rand32()) / UINT32_MAX;
}

#if GP_TYPE==float
  static inline GP_TYPE rand_num(void) { return rand_float(); }
#elif GP_TYPE==double
  static inline GP_TYPE rand_num(void) { return rand_double(); }
#else
  #error Invalid type specified for GP_TYPE
#endif

#endif
