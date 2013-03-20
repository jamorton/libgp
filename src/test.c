
//
// Contains methods for easily comparing world configurations and testing performance
//

#include "gp.h"

//
// `gp_test_configurations` will compare a set of world configurations by initializing
// a new world for each configuration in `confs`, running it for `iters` iterations,
// and recording the resulting world stats.
// Each configuration will be tested `num_times` times and the results will be averaged
//
void gp_test_configurations_iters(GpWorldConf * confs, uint count, uint iters, uint num_times)
{
	for (uint i = 0; i < count; i++)
	{
		gp_fitness_t best_tot = 0.0;
		gp_fitness_t avg_tot = 0.0;
		float len_tot = 0;

		setbuf(stdout, NULL);

		printf("Testing conf %d.", i + 1);

		for (uint j = 0; j < num_times; j++)
		{
			GpWorld * world = gp_world_new();
			gp_world_initialize(world, confs[i]);
			gp_world_evolve_times(world, iters);

			best_tot += world->stats.best_fitness;
			avg_tot  += world->stats.avg_fitness;
			len_tot  += world->stats.avg_program_length;

			gp_world_delete(world);

			printf(".");
		}

		printf(" Best Fit %-8.2f  Avg Fit %-8.2f  Avg length %-4.2f\n",
			best_tot / (gp_fitness_t)num_times,
			avg_tot / (gp_fitness_t)num_times,
			len_tot / (float)num_times);
	}
}

// Just like gp_test_configurations_iters except instead of running each
// world for a given number of iterations, they are run for `secs` seconds
void gp_test_configurations_secs(GpWorldConf * confs, uint count, float secs, uint num_times)
{
	for (uint i = 0; i < count; i++)
	{
		gp_fitness_t best_tot = 0.0;
		gp_fitness_t avg_tot = 0.0;
		float len_tot = 0;

		setbuf(stdout, NULL);

		printf("Testing conf %d.", i + 1);

		for (uint j = 0; j < num_times; j++)
		{
			GpWorld * world = gp_world_new();
			gp_world_initialize(world, confs[i]);
			gp_world_evolve_secs(world, secs);

			best_tot += world->stats.best_fitness;
			avg_tot  += world->stats.avg_fitness;
			len_tot  += world->stats.avg_program_length;

			gp_world_delete(world);

			printf(".");
		}

		printf(" Best Fit %-8.2f  Avg Fit %-8.2f  Avg length %-4.2f\n",
			best_tot / (gp_fitness_t)num_times,
			avg_tot / (gp_fitness_t)num_times,
			len_tot / (float)num_times);
	}
}

void gp_test_performance()
{

}
