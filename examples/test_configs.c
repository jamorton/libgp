
//
// Using the sqrt example, test different configurations against
// each other
//

#include "gp.h"
#include <math.h>

#define TEST_SIZE 100

typedef struct
{
	gp_num_t x;
	gp_num_t y;
} DataPoint;

DataPoint data[TEST_SIZE];

static gp_fitness_t eval(GpWorld * world, GpProgram * program)
{
	gp_fitness_t fit = 0;
	for (uint i = 0; i < TEST_SIZE; i++)
	{
		gp_num_t inp = data[i].x;
		GpState state = gp_program_run(world, program, &inp);

		const gp_num_t out = state.registers[0];
		const gp_num_t goal = data[i].y;
		const gp_num_t sqerr = gp_min((out - goal) * (out - goal), 999999999);

		fit += sqerr;
	}
	return sqrt(fit / TEST_SIZE);
}

static gp_num_t constant_func(void)
{
	return rand_num() * 10 - 5;
}

int main(void)
{
	// rand_num doesn't work until first world is initialized :(
	GpWorld * w = gp_world_new();

	for (int i = 0; i < TEST_SIZE; i++) {
		data[i].x = rand_num() * 10000;
		data[i].y = sqrt(data[i].x);
	}

	GpWorldConf default_conf = gp_world_conf_default();

	default_conf.constant_func      = &constant_func;
	default_conf.evaluator          = &eval;
	default_conf.population_size    = 50000;
	default_conf.num_inputs         = 1;
	default_conf.min_program_length = 5;
	default_conf.max_program_length = 30;
	default_conf.num_registers      = 2;
	default_conf.minimize_fitness   = 1;
	default_conf.mutate_rate        = 0.4;

	GpWorldConf confs[8];
	confs[0] = confs[1] = confs[2] = confs[3] = confs[4] = confs[5] = confs[6] = confs[7] = default_conf;

	confs[0].mutate_rate = 0.1;
	confs[1].mutate_rate = 0.25;
	confs[2].mutate_rate = 0.3;
	confs[3].mutate_rate = 0.35;
	confs[4].mutate_rate = 0.4;
	confs[5].mutate_rate = 0.5;
	confs[6].mutate_rate = 0.6;
	confs[7].mutate_rate = 0.7;

	gp_test_configurations_secs(confs, 8, 60, 3);
}
