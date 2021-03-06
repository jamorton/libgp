
// approximating the sqrt function with only +, -, *, and /.

#include "gp.h"
#include <math.h>

#define TEST_SIZE 150

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

		const gp_num_t err = state.registers[0] - data[i].y;
		const gp_num_t sqerr = gp_min(err * err, 999999999);

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
	GpWorld * world = gp_world_new();

	for (int i = 0; i < TEST_SIZE; i++) {
		data[i].x = rand_num() * 10000;
		data[i].y = sqrt(data[i].x);
	}

	GpWorldConf conf = gp_world_conf_default();
	conf.constant_func      = &constant_func;
	conf.evaluator          = &eval;
	conf.num_inputs         = 1;
	conf.minimize_fitness   = 1;

	gp_world_initialize(world, conf);

	uint loops = 0;
	uint total_steps = 0;
	float ips = 0.0;

	for (;;)
	{
		uint times = gp_world_evolve_secs(world, 1);
		ips = (ips * loops + times) / (loops + 1);
		total_steps += times;

		printf("Best: %-9.2f  Avg: %-9.2f  Gens: %-3u  Steps/sec: %-10.2f  Avg Len: %-5.2f\n",
			world->stats.best_fitness,
			world->stats.avg_fitness,
			world->stats.total_generations,
			ips,
			world->stats.avg_program_length);

		loops++;

		// Run for 60 seconds.
		if (loops > 60) {
			gp_program_export_python(stdout, world, &world->programs[0]);
			return 0;
		}
	}
}
