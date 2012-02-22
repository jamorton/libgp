
#ifndef __GP_H__
#define __GP_H__

#include <stdint.h>

#ifndef GP_NUM_REGISTERS
  #define GP_NUM_REGISTERS 2
#endif

#ifndef GP_TYPE
  #define GP_TYPE float
#endif

#ifndef GP_FITNESS_TYPE
  #define GP_FITNESS_TYPE double
#endif

typedef GP_TYPE gp_num_t;
typedef unsigned int uint;
typedef GP_FITNESS_TYPE gp_fitness_t;
typedef unsigned long long ulong;

#include "util.h"

typedef struct {
	gp_num_t registers[GP_NUM_REGISTERS];
	gp_num_t * inputs;
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
		gp_num_t num;
	} data;
} GpArg;

#include "ops.h"

typedef struct {
	uint output;
	GpArg args[GP_MAX_ARGS];
	GpOperation * op;
} GpStatement;

typedef struct {
	gp_fitness_t fitness;
	uint num_stmts;
	GpStatement * stmts;
} GpProgram;

typedef struct {
	GpProgram * programs;
	uint num_ops;
	GpOperation * ops;
	struct {
		gp_fitness_t (*evaluator)(GpProgram *);
		gp_num_t (*constant_func)(void);
		uint population_size;
		uint num_inputs;
		uint min_program_length;
		uint max_program_length;
		double mutation_rate;
		double elite_rate;
	} conf;
} GpWorld;

GpProgram * gp_program_new      (GpWorld *);
void        gp_program_delete   (GpProgram *);
void        gp_program_combine  (GpWorld *, GpProgram *, GpProgram *, GpProgram *);
GpState     gp_program_run      (GpWorld *, GpProgram *, gp_num_t *);
void        gp_program_debug    (GpProgram *);

GpWorld *   gp_world_new        (void);
void        gp_world_initialize (GpWorld *);
void        gp_world_add_op     (GpWorld *, GpOperation);
void        gp_world_evolve     (GpWorld *, uint);

#endif
