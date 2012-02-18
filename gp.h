
#ifndef __GP_H__
#define __GP_H__

#include <stdint.h>

#define HAVE_SSE2
#define MEXP 216091
#include "SFMT/SFMT.c"

#define GP_NUM_REGISTERS 2
#define GP_MIN_LENGTH 1
#define GP_MAX_LENGTH 5

#ifndef GP_TYPE
#define GP_TYPE unsigned int
#endif

typedef GP_TYPE gp_num;
typedef unsigned int uint;

typedef struct {
	gp_num registers[GP_NUM_REGISTERS];
	uint ip;
} GpState;

typedef void (*GpOperationFunc)(GpState *, gp_num **, gp_num *);

#include "ops.h"

typedef struct {
	gp_num * args[GP_NUM_REGISTERS];
	GpOperation * op;
} GpStatement;

typedef struct {
	uint num_stmts;
	GpStatement * stmts;
} GpProgram;

typedef struct {
	uint num_ops;
	GpOperation * ops;
} GpWorld;


/**
 * Generates a random number between low and high - 1
 *
 * @param low  lower bound (inclusive)
 * @param high upper bound (exclusive)
 */
static inline uint urand(uint low, uint high)
{
	return gen_rand32() % (high - low) + low;
}

GpProgram * gp_program_new(GpWorld *);
GpWorld * gp_world_new();
void gp_world_add_op(GpWorld *, GpOperation);
void gp_program_str(GpProgram *);

#endif
