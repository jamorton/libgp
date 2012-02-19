
#include "gp.h"
#include "mem.h"

#include <time.h>

static inline uint umin(uint a, uint b)
{
	return a < b ? a : b;
}

static GpStatement gp_random_statement(GpWorld * world)
{
	uint j;
	GpStatement stmt;
	stmt.output = urand(0, GP_NUM_REGISTERS);
	stmt.op = &world->ops[urand(0, world->num_ops)];
	for (j = 0; j < stmt.op->num_args; j++)
	{
		switch (urand(0, 3))
		{
		case 0:
			stmt.args[j].type = GP_ARG_REGISTER;
			stmt.args[j].data.reg = urand(0, GP_NUM_REGISTERS);
			break;
		case 1:
			stmt.args[j].type = GP_ARG_CONSTANT;
			stmt.args[j].data.num = gen_rand32();
			break;
		case 2:
			stmt.args[j].type = GP_ARG_INPUT;
			stmt.args[j].data.num = urand(0, world->num_inputs);
			break;
		}
	}
	return stmt;
}

GpProgram * gp_program_new(GpWorld * world)
{
	uint i, j;
	GpProgram * program = new(GpProgram);
	program->num_stmts = urand(GP_MIN_LENGTH, GP_MAX_LENGTH);
	program->stmts = new_array(GpStatement, program->num_stmts);
	for (i = 0; i < program->num_stmts; i++)
		program->stmts[i] = gp_random_statement(world);
	return program;
}

GpProgram * gp_program_combine(GpWorld * world, GpProgram * mom, GpProgram * dad)
{
	uint i;
	GpProgram * children = new_array(GpProgram, 2);
	uint cp = urand(0, umin(mom->num_stmts, dad->num_stmts) + 1);

	children[0].num_stmts = dad->num_stmts;
	children[1].num_stmts = mom->num_stmts;
	children[0].stmts = new_array(GpStatement, children[0].num_stmts);
	children[1].stmts = new_array(GpStatement, children[1].num_stmts);
		
	for (i = 0; i < cp; i++)
	{
		children[0].stmts[i] = mom->stmts[i];
		children[1].stmts[i] = dad->stmts[i];
	}

	for (i = cp; i < dad->num_stmts; i++)
		children[0].stmts[i] = dad->stmts[i];
	for (i = cp; i < mom->num_stmts; i++)
		children[1].stmts[i] = mom->stmts[i];

	while (drand() < GP_MUTATE_RATE)
		children[0].stmts[urand(0, children[0].num_stmts)] = gp_random_statement(world);
	while (drand() < GP_MUTATE_RATE)
		children[1].stmts[urand(0, children[1].num_stmts)] = gp_random_statement(world);

	return children;
}

void gp_program_debug(GpProgram * program)
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
	
	GpProgram * mom = gp_program_new(world);
	GpProgram * dad = gp_program_new(world);

	printf("mom\n");
	gp_program_debug(mom);
	printf("\ndad\n");
	gp_program_debug(dad);
	
	GpProgram * children = gp_program_combine(world, mom, dad);
	printf("\nchild 1\n");
	gp_program_debug(&children[0]);
	printf("\nchild 2\n");
	gp_program_debug(&children[1]);
}
