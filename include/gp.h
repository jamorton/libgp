
#ifndef GP_INCLUDE_GP_H
#define GP_INCLUDE_GP_H

#include "common.h"

//
// ### Structures ###
//

// Program Structures

typedef enum {
	GP_ARG_REGISTER = 0,
	GP_ARG_CONSTANT,
	GP_ARG_COUNT
} GpArgType;

struct GpState_ {
	gp_num_t registers[GP_MAX_REGISTERS];
	uint ip;
};

struct GpArg_ {
	GpArgType type;
	union {
		uint reg;
		gp_num_t num;
	} data;
};

typedef struct GpState_ GpState;
typedef struct GpArg_ GpArg;

// This needs to be included exactly here
#include "ops.h"

struct GpStatement_ {
	uint output;
	GpArg args[GP_MAX_ARGS];
	GpOperation * op;
};

struct GpProgram_ {
	gp_fitness_t fitness;
	int evaluated;
	uint num_stmts;
	struct GpStatement_ * stmts;
};

// World Structures

struct GpWorld_;
typedef struct GpWorld_ GpWorld;
typedef struct GpStatement_ GpStatement;
typedef struct GpProgram_ GpProgram;

typedef struct GpWorldConf_ {
	GpOperation * ops;
	uint num_ops;
	gp_fitness_t (*evaluator)(GpWorld *, GpProgram *);
	gp_num_t (*constant_func)(void);
	uint population_size;
	uint num_inputs;
	uint num_registers;
	uint min_program_length;
	uint max_program_length;
	float mutate_rate;
	float crossover_rate;
	float homologous_rate;
	int minimize_fitness;
	int auto_optimize;
} GpWorldConf;

struct GpWorld_ {
	GpProgram * programs;
	GpWorldConf conf;
	uint num_ops;
	int has_init;

	struct {
		gp_fitness_t avg_fitness;
		gp_fitness_t best_fitness;
		uint total_steps;
		uint total_generations;
		float avg_program_length;
	} stats;

	// private
	GpStatement * _stmt_buf;
	uint _last_optimize;
};

//
// ### Function prototypes ###
//

// Program-related functions
GpStatement gp_statement_random      (GpWorld *);
GpProgram * gp_program_new           (GpWorld *);
void        gp_program_init          (GpWorld *, GpProgram *);
void        gp_program_copy          (GpProgram *, GpProgram *);
void        gp_program_delete        (GpProgram *);
int         gp_program_equal         (GpProgram *, GpProgram *);
GpState     gp_program_run           (GpWorld *, GpProgram *, gp_num_t *);
void        gp_program_print         (FILE *, GpProgram *);
void        gp_program_export_python (FILE *, GpWorld *, GpProgram *);

// World-related functions
GpWorld *   gp_world_new           (void);
void        gp_world_delete        (GpWorld *);
void        gp_world_initialize    (GpWorld *, GpWorldConf);
GpWorldConf gp_world_conf_default  (void);
void        gp_world_evolve_times  (GpWorld *, uint);
uint        gp_world_evolve_secs   (GpWorld *, float);
void        gp_world_evolve_gens   (GpWorld *, uint);
void        gp_world_optimize      (GpWorld *);

// Evolutionary operators
void        gp_mutate           (GpWorld *, GpProgram *);
void        gp_cross_homologous (GpProgram *, GpProgram *, GpProgram *, GpProgram *);
void        gp_cross_twopoint   (GpWorld *, GpProgram *, GpProgram *, GpProgram *);

// Testing functions
void        gp_world_optimize_test (void);
void        gp_test_configurations (GpWorldConf *, uint, uint, uint);
void        gp_test_performance    (void);

#endif
