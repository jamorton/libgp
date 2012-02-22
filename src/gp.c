
#include "gp.h"
#include "mem.h"
#include "iqsort.h"

#include <time.h>

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

static void gp_program_init(GpWorld * world, GpProgram * program)
{
	uint i;
	program->num_stmts = urand(world->conf.min_program_length, world->conf.max_program_length);
	program->stmts = new_array(GpStatement, program->num_stmts);
	for (i = 0; i < program->num_stmts; i++)
		program->stmts[i] = gp_random_statement(world);
	return program;
}

GpProgram * gp_program_new(GpWorld * world)
{
	GpProgram * program = new(GpProgram);
	gp_program_init(world, program);
	return program;
}

void gp_program_delete(GpProgram * program)
{
	delete(program->stmts);
}

void gp_program_combine(GpWorld * world, GpProgram * mom, GpProgram * dad, GpProgram ** children)
{
	uint i;
	uint cp = urand(0, umin(mom->num_stmts, dad->num_stmts) + 1);

	children[0] = new(GpProgram);
	children[1] = new(GpProgram);
	children[0]->num_stmts = dad->num_stmts;
	children[1]->num_stmts = mom->num_stmts;
	children[0]->stmts = new_array(GpStatement, children[0]->num_stmts);
	children[1]->stmts = new_array(GpStatement, children[1]->num_stmts);
	
	for (i = 0; i < cp; i++)
	{
		children[0]->stmts[i] = mom->stmts[i];
		children[1]->stmts[i] = dad->stmts[i];
	}

	for (i = cp; i < dad->num_stmts; i++)
		children[0]->stmts[i] = dad->stmts[i];
	for (i = cp; i < mom->num_stmts; i++)
		children[1]->stmts[i] = mom->stmts[i];

	while (drand() < world->conf.mutation_rate)
		children[0]->stmts[urand(0, children[0]->num_stmts)] = gp_random_statement(world);
	while (drand() < world->conf.mutation_rate)
		children[1]->stmts[urand(0, children[1]->num_stmts)] = gp_random_statement(world);
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

static char gp_has_init = 0;

GpWorld * gp_world_new()
{
	if (!gp_has_init)
	{
		init_gen_rand(time(NULL));
		gp_has_init = 1;
	}
	GpWorld * world = new(GpWorld);
	world->num_ops = 0;
	world->ops = NULL;
	world->programs = NULL;

	world->conf.population_size = 10000;
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
	world->programs = new_array(GpProgram, world->conf.population_size);
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

static inline GpProgram * roulette_select(GpWorld * world, ulong total_fitness)
{
	ulong targ = lrand(0, total_fitness);
	ulong cur = 0;
	uint cur_idx = 0;
	
	while (cur < targ)
		cur += world->programs[cur_idx++].fitness;
	return world->programs + (cur_idx - 1);
}

static inline GpProgram * tournament_select(GpWorld * world)
{
	const uint k = world->conf.population_size;

	GpProgram * p1 = world->programs + urand(0, k);
	GpProgram * p2 = world->programs + urand(0, k);
	GpProgram * p3 = world->programs + urand(0, k);
	GpProgram * p4 = world->programs + urand(0, k);

	uint most = p1->fitness;
	GpProgram * most_prog = p1;

	if (p2->fitness > most) { most = p2->fitness; most_prog = p2; }
	if (p3->fitness > most) { most = p3->fitness; most_prog = p3; }
	if (p4->fitness > most) { most = p4->fitness; most_prog = p4; }

	return most_prog;
}

static inline void gp_world_evolve_step(GpWorld * world)
{
	if ((world->conf.population_size & 1) != 0)
	{
		printf("ERROR: population_size must be even");
		abort();
	}

	uint i;
	ulong total_fitness = 0ULL;
	uint popsize = world->conf.population_size;
	
	for (i = 0; i < popsize; i++)
	{
		world->programs[i]->fitness = world->conf.evaluator(world->programs[i]);
		total_fitness += world->programs[i]->fitness;
	}

#define _CMP(a,b) ((*a)->fitness > (*b)->fitness)
	QSORT(GpProgram *, world->programs, popsize, _CMP);
#undef _CMP

	GpProgram * new_programs[popsize];
	/* make elite rate even so the two-children-per-loop thing below works out */
	uint elites = (uint)(popsize * world->conf.elite_rate) & ~1;

	for (i = 0; i < elites; i++)
		new_programs[i] = world->programs[i];

	for (i = elites; i < popsize; i += 2)
	{
		GpProgram * mom = tournament_select(world);
		GpProgram * dad = tournament_select(world);

		GpProgram * children[2];
		gp_program_combine(world, mom, dad, children);
		new_programs[i]     = children[0];
		new_programs[i + 1] = children[1];
	}

	for (i = elites; i < popsize; i++)
		gp_program_delete(world->programs[i]);

	memcpy(world->programs, new_programs, sizeof(GpProgram *) * popsize);
}

void gp_world_evolve(GpWorld * world, uint times)
{
	while (times--)
		gp_world_evolve_step(world);
}
