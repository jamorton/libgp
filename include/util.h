
#ifndef __GP_UTIL_H__
#define __GP_UTIL_H__

#define HAVE_SSE2 1
#define MEXP 19937
#include "SFMT/SFMT.h"

static inline uint umin(uint a, uint b)
{
	return a < b ? a : b;
}

static inline ulong lrand(ulong low, ulong high)
{
	return ((ulong)gen_rand32() << 32 | (ulong)gen_rand32()) % (high - low) + low;
}

static inline uint urand(uint low, uint high)
{
	return gen_rand32() % (high - low) + low;
}

static inline double drand(void)
{
	return (double)(gen_rand32()) / UINT32_MAX;
}

static inline float frand(void)
{
	return (float)(gen_rand32()) / UINT32_MAX;
}


#endif
