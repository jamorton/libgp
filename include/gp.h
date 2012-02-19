
#ifndef __GP_H__
#define __GP_H__

#include <stdint.h>

#define HAVE_SSE2
#define MEXP 216091
#include "SFMT.c"

#ifndef GP_NUM_REGISTERS
#define GP_NUM_REGISTERS 2
#endif

#define GP_MIN_LENGTH 1
#define GP_MAX_LENGTH 5

#ifndef GP_TYPE
#define GP_TYPE unsigned int
#endif

#ifndef GP_MUTATE_RATE
#define GP_MUTATE_RATE 0.01
#endif

typedef GP_TYPE gp_num;
typedef unsigned int uint;

typedef struct {
	gp_num registers[GP_NUM_REGISTERS];
	uint ip;
} GpState;

typedef void (*GpOperationFunc)(GpState *, gp_num **, gp_num *);

#include "ops.h"

typedef enum {
	GP_ARG_REGISTER = 0,
	GP_ARG_CONSTANT,
	GP_ARG_INPUT,
	GP_ARG_COUNT
} GpArgType;

typedef struct {
	GpArgType type;
	union {
		uint reg;
		gp_num num;
	} data;
} GpArg;

typedef struct {
	uint output;
	GpArg args[GP_MAX_ARGS];
	GpOperation * op;
} GpStatement;

typedef struct {
	uint num_stmts;
	GpStatement * stmts;
} GpProgram;

typedef struct {
	uint num_ops;
	uint num_inputs;
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

static inline double drand(void)
{
	return (double)(gen_rand32()) / UINT32_MAX;
}

GpProgram * gp_program_new(GpWorld *);
GpProgram * gp_program_combine(GpWorld *, GpProgram *, GpProgram *);
void gp_program_debug(GpProgram *);
GpWorld * gp_world_new(void);
void gp_world_add_op(GpWorld *, GpOperation);


#endif
