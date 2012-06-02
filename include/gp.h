
#ifndef __GP_H__
#define __GP_H__

#include <stdint.h>

#ifndef GP_MAX_REGISTERS
  #define GP_MAX_REGISTERS 5
#endif

#ifndef GP_TYPE
  #define GP_TYPE float
#endif

typedef GP_TYPE gp_num_t;
typedef unsigned int uint;
typedef double gp_fitness_t;
typedef unsigned long long ulong;

#include "util.h"

typedef struct {
	gp_num_t registers[GP_MAX_REGISTERS];
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

// This needs to be included exactly here
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

typedef struct GpWorld {

	GpProgram * programs;
	uint num_ops;
	GpOperation * ops;

	struct {
		gp_fitness_t (*evaluator)(struct GpWorld *, GpProgram *);
		gp_num_t (*constant_func)(void);
		uint population_size;
		uint num_inputs;
		uint min_program_length;
		uint max_program_length;
		uint num_registers;
		double mutate_rate;
        double crossover_rate;
	} conf;

	struct {
		gp_fitness_t avg_fitness;
		gp_fitness_t best_fitness;
	} data;

} GpWorld;

GpProgram * gp_program_new      (GpWorld *);
void        gp_program_copy     (GpProgram *, GpProgram *);
void        gp_program_delete   (GpProgram *);
int         gp_program_equal    (GpProgram *, GpProgram *);
GpState     gp_program_run      (GpWorld *, GpProgram *, gp_num_t *);
void        gp_program_debug    (GpProgram *);


GpWorld *   gp_world_new         (void);
void        gp_world_initialize  (GpWorld *);
void        gp_world_add_op      (GpWorld *, GpOperation);
void        gp_world_evolve      (GpWorld *, uint);
uint        gp_world_evolve_secs (GpWorld *, uint);

void        gp_mutate           (GpWorld *, GpProgram *);
void        gp_cross_homologous (GpProgram *, GpProgram *, GpProgram *);

#endif
