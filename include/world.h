
#ifndef GP_INCLUDE_WORLD_H
#define GP_INCLUDE_WORLD_H

#include "program.h"

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
		float mutate_rate;
		float crossover_rate; // UNUSED
		int minimize_fitness;
	} conf;

	struct {
		gp_fitness_t avg_fitness;
		gp_fitness_t best_fitness;
		uint total_steps;
	} data;

	struct {
		gp_fitness_t total_fitness;
	} _private;

} GpWorld;

GpWorld *   gp_world_new         (void);
void        gp_world_initialize  (GpWorld *);
void        gp_world_add_op      (GpWorld *, GpOperation);
void        gp_world_evolve      (GpWorld *, uint);
uint        gp_world_evolve_secs (GpWorld *, uint);

#endif
