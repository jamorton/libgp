
#include "gp.h"
#include "mem.h"

#include <stdint.h>
#include <time.h>

GpProgram * gp_program_new(GpWorld * world)
{
	uint i, j;
	GpProgram * program = new(GpProgram);
	program->num_stmts = urand(GP_MIN_LENGTH, GP_MAX_LENGTH);
	program->stmts = new_array(GpStatement, program->num_stmts);
	for (i = 0; i < program->num_stmts; i++)
	{
		GpStatement * stmt = &program->stmts[i];
		stmt->output = urand(0, GP_NUM_REGISTERS);
		stmt->op = &world->ops[urand(0, world->num_ops)];
		for (j = 0; j < stmt->op->num_args; j++)
		{
			switch (urand(0, 3))
			{
			case 0:
				stmt->args[j].type = GP_ARG_REGISTER;
				stmt->args[j].data.reg = urand(0, GP_NUM_REGISTERS);
				break;
			case 1:
				stmt->args[j].type = GP_ARG_CONSTANT;
				stmt->args[j].data.num = gen_rand32();
				break;
			case 2:
				stmt->args[j].type = GP_ARG_INPUT;
				stmt->args[j].data.num = urand(0, world->num_inputs);
				break;
			}
		}
	}
	return program;
}

void gp_program_str(GpProgram * program)
{
	uint i, j;
	for (i = 0; i < program->num_stmts; i++)
	{
		GpStatement * stmt = &program->stmts[i];
		printf("r%u = %s ", stmt->output, stmt->op->name);
		for (j = 0; j < stmt->op->num_args; j++)
		{
			switch (stmt->args[j].type)
			{
			case GP_ARG_REGISTER:
				printf("r%d", stmt->args[j].data.reg);
				break;
			case GP_ARG_CONSTANT:
				printf("%u", stmt->args[j].data.num);
				break;
			case GP_ARG_INPUT:
				printf("i%u", stmt->args[j].data.num);
				break;
			default:
				printf("<unknown>");
			}
			if (j != stmt->op->num_args - 1)
				printf(", ");
		}
		printf("\n");
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

	world->num_inputs = 1;
	
	gp_world_add_op(world, GP_OP(add));
	gp_world_add_op(world, GP_OP(mul));
	
	GpProgram * program = gp_program_new(world);
	gp_program_str(program);
}
