
#ifndef __GP_UTIL_H__
#define __GP_UTIL_H__

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
