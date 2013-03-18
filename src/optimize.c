
#include "gp.h"

#include <string.h>

static uint _remove_introns(GpWorld * world, GpProgram * program)
{
	int used_vars[GP_MAX_REGISTERS];
	int marked[program->num_stmts];

	memset(used_vars, 0, sizeof(int) * GP_MAX_REGISTERS);
	memset(marked, 0, sizeof(int) * program->num_stmts);

	used_vars[0] = 1;

	// Detect introns. A statement `i` is an intron if
	// at the end of this loop marked[i] is 0.
	for (int i = program->num_stmts - 1; i >= 0; i--)
	{
		GpStatement * stmt = &program->stmts[i];
		uint out = stmt->output;
		if (used_vars[out])
		{
			marked[i] = 1;
			used_vars[out] = 0;
			for (uint j = 0; j < stmt->op->num_args; j++)
				if (stmt->args[j].type == GP_ARG_REGISTER)
					used_vars[stmt->args[j].data.reg] = 1;
		}
	}

	// compact the statement list together so all introns are removed
	uint idx = 0;
	for (uint i = 0; i < program->num_stmts; i++)
	{
		if (marked[i] || (int)program->num_stmts - (int)i <= (int)world->conf.min_program_length - (int)idx)
			program->stmts[idx++] = program->stmts[i];
	}

	if (idx < world->conf.min_program_length)
		printf("BADBABDA %d  %d  originally %d\n", idx, world->conf.min_program_length, program->num_stmts);

	uint num_introns = program->num_stmts - idx;
	program->num_stmts = idx;

	return num_introns;
}

void gp_world_optimize(GpWorld * world)
{
	uint introns_removed = 0;
	for (uint i = 0; i < world->conf.population_size; i++)
		introns_removed += _remove_introns(world, &world->programs[i]);

}

//
// ## Testing Optimization Functions ##
//

#define TEST_SIZE 100

gp_num_t _test_data[TEST_SIZE];
gp_num_t _test_out[TEST_SIZE];

static gp_num_t _test_constant_func(void)
{
	return rand_num() * 10 - 5;
}

static gp_fitness_t _test_eval(GpWorld * world, GpProgram * program)
{
	return 0;
}

void gp_world_optimize_test()
{

	GpWorld * world = gp_world_new();

	for (int i = 0; i < TEST_SIZE; i++)
		_test_data[i] = rand_num() * 10000;

	gp_world_add_op(world, gp_op_add);
	gp_world_add_op(world, gp_op_mul);
	gp_world_add_op(world, gp_op_sub);
	gp_world_add_op(world, gp_op_div);
	gp_world_add_op(world, gp_op_eq);

	GpWorldConf conf = gp_world_conf_default();
	conf.num_registers = 3;
	conf.constant_func = &_test_constant_func;
	conf.evaluator = &_test_eval;
	conf.num_inputs = 1;

	gp_world_initialize(world, conf);

	uint i, j;

	for (i = 0; i < world->conf.population_size; i++)
	{
		for (j = 0; j < TEST_SIZE; j++)
		{
			GpState state = gp_program_run(world, &world->programs[i], _test_data + j);
			_test_out[j] = state.registers[0];
		}
		_remove_introns(world, &world->programs[i]);
		for (j = 0; j < TEST_SIZE; j++)
		{
			GpState state = gp_program_run(world, &world->programs[i], _test_data + j);
			if (state.registers[0] != _test_out[j])
				printf("ERROR! Intron removal changed program output: %f vs %f\n",
					state.registers[0], _test_out[j]);
		}
	}

	gp_world_delete(world);
}
