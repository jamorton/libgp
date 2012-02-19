
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

#ifndef GP_POPULATION_SIZE
#define GP_POPULATION_SIZE 100000
#endif

#ifndef GP_NUM_INPUTS
#define GP_NUM_INPUTS 1
#endif

#ifndef GP_CONSTANT_FUNC
#define GP_CONSTANT_FUNC gen_rand32
#endif

typedef GP_TYPE gp_num;
typedef unsigned int uint;

typedef struct {
	gp_num registers[GP_NUM_REGISTERS];
	gp_num * inputs;;
	uint ip;
	void * data;
} GpState;

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

#include "ops.h"

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
	GpOperation * ops;
	GpProgram * programs[GP_POPULATION_SIZE];
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
GpState gp_program_run(GpWorld *, GpProgram *, gp_num[GP_NUM_INPUTS]);
void gp_world_initialize(GpWorld *);

void gp_program_debug(GpProgram *);

GpWorld * gp_world_new(void);
void gp_world_add_op(GpWorld *, GpOperation);

#endif
