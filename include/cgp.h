
#ifndef GP_INCLUDE_CGP_H
#define GP_INCLUDE_CGP_H

#include "common.h"

//
// ### Structures ###
//

// Program Structures

struct GpState_t {
	gp_num_t registers[GP_MAX_REGISTERS];
	gp_num_t * inputs;
	uint ip;
	void * data;
};

typedef enum {
	GP_ARG_REGISTER = 0,
	GP_ARG_CONSTANT,
	GP_ARG_INPUT,
	GP_ARG_COUNT
} GpArgType;

struct GpArg_t {
	GpArgType type;
	union {
		uint reg;
		gp_num_t num;
	} data;
};

typedef struct GpState_t GpState;
typedef struct GpArg_t GpArg;

// This needs to be included exactly here
#include "ops.h"

struct GpStatement_t {
	uint output;
	GpArg args[GP_MAX_ARGS];
	GpOperation * op;
};

struct GpProgram_t {
	gp_fitness_t fitness;
	int evaluated;
	uint num_stmts;
	struct GpStatement_t * stmts;
};

// World Structures

struct GpWorld_t;
typedef struct GpWorld_t GpWorld;
typedef struct GpStatement_t GpStatement;
typedef struct GpProgram_t GpProgram;

typedef struct GpWorldConf_t {
	gp_fitness_t (*evaluator)(GpWorld *, GpProgram *);
	gp_num_t (*constant_func)(void);
	uint population_size;
	uint num_inputs;
	uint num_registers;
	uint min_program_length;
	uint max_program_length;
	float mutate_rate;
	float crossover_rate;
	int minimize_fitness;
} GpWorldConf;

struct GpWorld_t {
	GpProgram * programs;
	GpWorldConf conf;
	uint num_ops;
	GpOperation * ops;
	int has_init;

	struct {
		gp_fitness_t avg_fitness;
		gp_fitness_t best_fitness;
		uint total_steps;
		uint total_generations;
	} stats;

	// private
	GpStatement * _stmt_buf;
};

//
// ### Function prototypes ###
//

// Program-related functions
GpStatement gp_statement_random (GpWorld *);
GpProgram * gp_program_new      (GpWorld *);
void        gp_program_init     (GpWorld *, GpProgram *);
void        gp_program_copy     (GpProgram *, GpProgram *);
void        gp_program_delete   (GpProgram *);
int         gp_program_equal    (GpProgram *, GpProgram *);
GpState     gp_program_run      (GpWorld *, GpProgram *, gp_num_t *);
void        gp_program_print    (FILE *, GpProgram *);

void        gp_program_export_python (FILE *, GpWorld *, GpProgram *);

// World-related functions
GpWorld *   gp_world_new          (void);
void        gp_world_initialize   (GpWorld *, GpWorldConf);
GpWorldConf gp_world_conf_default (void);
void        gp_world_add_op       (GpWorld *, GpOperation);
void        gp_world_evolve       (GpWorld *, uint);
uint        gp_world_evolve_secs  (GpWorld *, uint);

// Evolution functions
void        gp_mutate           (GpWorld *, GpProgram *);
void        gp_cross_homologous (GpProgram *, GpProgram *, GpProgram *, GpProgram *);
void        gp_cross_twopoint   (GpWorld *, GpProgram *, GpProgram *, GpProgram *);

#endif
