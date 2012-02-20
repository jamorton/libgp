
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

	uint randopt = world->conf.num_inputs == 0 ? 2 : 3;

	for (j = 0; j < stmt.op->num_args; j++)
	{
		switch (urand(0, randopt))
		{
		case 0:
			stmt.args[j].type = GP_ARG_REGISTER;
			stmt.args[j].data.reg = urand(0, GP_NUM_REGISTERS);
			break;
		case 1:
			stmt.args[j].type = GP_ARG_CONSTANT;
			stmt.args[j].data.num = GP_CONSTANT_FUNC();
			break;
		case 2:
			stmt.args[j].type = GP_ARG_INPUT;
			stmt.args[j].data.reg = urand(0, world->conf.num_inputs);
			break;
		}
	}
	return stmt;
}

GpProgram * gp_program_new(GpWorld * world)
{
	uint i;
	GpProgram * program = new(GpProgram);
	program->num_stmts = urand(world->conf.min_program_length, world->conf.max_program_length);
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

	while (drand() < world->conf.mutation_rate)
		children[0].stmts[urand(0, children[0].num_stmts)] = gp_random_statement(world);
	while (drand() < world->conf.mutation_rate)
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

GpState gp_program_run(GpWorld * word, GpProgram * program, gp_num_t * inputs)
{
	uint i;
	GpState state;
	state.ip = 0;
	memset(state.registers, 0, GP_NUM_REGISTERS * sizeof(gp_num_t));
	state.inputs = inputs;
	while (state.ip < program->num_stmts)
	{
		GpStatement * stmt = &program->stmts[state.ip];
		(stmt->op->func)(&state, stmt->args, state.registers + stmt->output);
		state.ip++;
	}
	return state;
}


/********* GpWorld *********/

GpWorld * gp_world_new()
{
	GpWorld * world = new(GpWorld);
	world->num_ops = 0;
	world->ops = NULL;
	world->programs = NULL;

	world->conf.population_size = 100;
	world->conf.num_inputs = 0;
	world->conf.min_program_length = 1;
	world->conf.max_program_length = 5;
	world->conf.mutation_rate = 0.01;
	world->conf.elite_rate = 0.05;

	return world;
}

void gp_world_initialize(GpWorld * world)
{
	uint i;
	world->programs = new_array(GpProgram *, world->conf.population_size);
	for (i = 0; i < world->conf.population_size; i++)
		world->programs[i] = gp_program_new(world);
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

static int fitness_compare(const void * a, const void * b)
{
	const GpProgram * _a = *(const GpProgram **)a;
	const GpProgram * _b = *(const GpProgram **)b;
	if (_a->fitness > _b->fitness)
		return 1;
	else if (_a->fitness < _b->fitness)
		return -1;
	return 0;
}

static inline void gp_world_evolve_step(GpWorld * world)
{
	uint i;
	unsigned long long total_fitness = 0ULL;
	for (i = 0; i < world->conf.population_size; i++)
	{
		world->programs[i]->fitness = world->conf.evaluator(world->programs[i]);
		total_fitness += world->programs[i]->fitness;
	}

	qsort(world->programs, world->conf.population_size, sizeof(GpProgram *), &fitness_compare);

	
	
}

void gp_world_evolve(GpWorld * world, uint times)
{
	while (times--)
		gp_world_evolve_step(world);
}

static gp_fitness_t test_fit(GpProgram * program)
{
	return urand(0, 100000);
}

int main(void)
{
	uint i;
	init_gen_rand(time(NULL));
	
	GpWorld * world = gp_world_new();

	gp_world_add_op(world, GP_OP(add));
	gp_world_add_op(world, GP_OP(mul));
	gp_world_add_op(world, GP_OP(eq));
	gp_world_add_op(world, GP_OP(xor));

	world->conf.evaluator = &test_fit;

	gp_world_initialize(world);

	gp_world_evolve(world, 1);

}
