
#include "gp.h"

static gp_fitness_t test_fit(GpProgram * program)
{
	return drand();
}

static gp_num_t constant_func(void)
{
	return urand(0, 10000);
}

int main(void)
{
	
	GpWorld * world = gp_world_new();

	gp_world_add_op(world, GP_OP(add));
	gp_world_add_op(world, GP_OP(mul));
	gp_world_add_op(world, GP_OP(eq));
	gp_world_add_op(world, GP_OP(xor));

	world->conf.constant_func = &constant_func;
	world->conf.evaluator = &test_fit;
	world->conf.population_size = 20000;

	gp_world_initialize(world);

	gp_world_evolve(world, 1000);
}

