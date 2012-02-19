
#include "gp.h"
#include "mem.h"

#include <stdint.h>
#include <time.h>

GpProgram * gp_program_new(GpWorld * world)
{
	uint i;
	GpProgram * program = new(GpProgram);
	program->num_stmts = urand(GP_MIN_LENGTH, GP_MAX_LENGTH);
	program->stmts = new_array(GpStatement, program->num_stmts);
	for (i = 0; i < program->num_stmts; i++)
	{
		program->stmts[i].op = &world->ops[urand(0, world->num_ops)];
	}
	return program;
}

void gp_program_str(GpProgram * program)
{
	uint i;
	for (i = 0; i < program->num_stmts; i++)
	{
		printf("%s\n", program->stmts[i].op->name);
	}
}

GpWorld * gp_world_new()
{
	GpWorld * world = new(GpWorld);
	world->num_ops = 0;
	world->ops = NULL;
	return world;
}

void gp_world_add_op(GpWorld * world, GpOperation op)
{
	if (world->num_ops == 0)
		world->ops = new(GpOperation);
	else
		world->ops = realloc(world->ops, sizeof(GpOperation) * (world->num_ops + 1));

	world->ops[world->num_ops] = op;
	world->num_ops++;
}

int main(void)
{
	init_gen_rand(time(NULL));
	
	GpWorld * world = gp_world_new();
	gp_world_add_op(world, GP_OP(add));
	gp_world_add_op(world, GP_OP(mul));

	
	GpProgram * program = gp_program_new(world);
	gp_program_str(program);
}
