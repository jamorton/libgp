
// **gp.c** contains most of cgp's main functionality, including
// allocation and setup for the world and program structures, as well
// as evolution logic, genetic selectors, and genetic operators

#include "gp.h"
#include "mem.h"
#include "iqsort.h"

#include "SFMT/SFMT.c"

#include <time.h>

//                 Init and Utility Functions
// --------------------------------------------------------------

// ### GpWorld  ###
static void gp_program_init(GpWorld *, GpProgram *);
static char gp_has_init = 0;

// `gp_world_new` creates a new world with a default config.
// After a world is created, custom configuration should be set and
// the desired list of operations should be added with `gp_world_add_op`.
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

	// Configuration defaults. Override these before calling `gp_world_initialize`
	world->conf.population_size    = 10000;
	world->conf.num_registers      = 2;
	world->conf.num_inputs         = 0;
	world->conf.min_program_length = 1;
	world->conf.max_program_length = 5;
	world->conf.crossover_rate     = 0.90;
	world->conf.mutate_rate        = 0.10;
	world->conf.evaluator          = NULL;
	world->conf.constant_func      = NULL;

	return world;
}

static void init_err(const char * estr)
{
	printf("libgp init ERROR: %s\n", estr);
	abort();
}

// `gp_world_initialize` should be called after the world has been
// created and completely configured. No configuration should be changed
// after calling this.
void gp_world_initialize(GpWorld * world)
{
	if (world->conf.constant_func == NULL || world->conf.evaluator == NULL)
		init_err("constant_func or evaluator not defined");

	if (world->conf.num_registers > GP_MAX_REGISTERS)
		init_err("num_registers is greater than GP_MAX_REGISTERS");

	if (world->conf.min_program_length < 3)
		init_err("min_program_length must be 3 or greater");

	if (world->conf.max_program_length < world->conf.min_program_length)
		init_err("max_program_length cannot be greater than min_program_length");

	if ((world->conf.population_size & 1) != 0)
		init_err("population size must be an even number");

	if (world->conf.mutate_rate + world->conf.crossover_rate > 1.0)
		init_err("mutate_rate + crossover_rate cannot be greater than 1");

	uint i;
	world->programs = new_array(GpProgram, world->conf.population_size);
	for (i = 0; i < world->conf.population_size; i++)
		gp_program_init(world, world->programs + i);
}

// `gp_world_add_op` is used to  make an operation available for
// use in programs. For example:
//
//     gp_world_add_op(world, GP_OP(add));
//     gp_world_add_op(world, GP_OP(sub));
//
// A list of builtin operators can be found in **ops.h**
void gp_world_add_op(GpWorld * world, GpOperation op)
{
	if (world->num_ops == 0)
		world->ops = new(GpOperation);
	else
		world->ops = realloc(world->ops, sizeof(GpOperation) * (world->num_ops + 1));

	world->ops[world->num_ops] = op;
	world->num_ops++;
}

// ### GpStatement  ###
//
// A statement is created as a random operation with random arguments.
// Each argument can be a register, constant, or input.
static GpStatement gp_random_statement(GpWorld * world)
{
	uint j;

	GpStatement stmt;
	stmt.output = urand(0, world->conf.num_registers);
	stmt.op = &world->ops[urand(0, world->num_ops)];

	uint randopt = world->conf.num_inputs == 0 ? 2 : 3;

	for (j = 0; j < stmt.op->num_args; j++)
	{
		switch (urand(0, randopt))
		{
		case 0:
			stmt.args[j].type = GP_ARG_REGISTER;
			stmt.args[j].data.reg = urand(0, world->conf.num_registers);
			break;
		case 1:
			stmt.args[j].type = GP_ARG_CONSTANT;
			stmt.args[j].data.num = world->conf.constant_func();
			break;
		case 2:
			stmt.args[j].type = GP_ARG_INPUT;
			stmt.args[j].data.reg = urand(0, world->conf.num_inputs);
			break;
		}
	}
	return stmt;
}

//
// ### GpProgram  ###
//
// Create a program with a random length N and a sequence of N randomly
// initialized statements
static void gp_program_init(GpWorld * world, GpProgram * program)
{
	uint i;
	program->num_stmts = urand(world->conf.min_program_length,
							   world->conf.max_program_length);
	program->stmts = new_array(GpStatement, program->num_stmts);
	for (i = 0; i < program->num_stmts; i++)
		program->stmts[i] = gp_random_statement(world);
}

GpProgram * gp_program_new(GpWorld * world)
{
	GpProgram * program = new(GpProgram);
	gp_program_init(world, program);
	return program;
}

// Copy all of one program's satements and data to another
// **Does NOT** free `dst`'s stmts.
void gp_program_copy(GpProgram * src, GpProgram * dst)
{
	dst->num_stmts = src->num_stmts;
	dst->stmts = new_array(GpStatement, dst->num_stmts);
	memcpy(dst->stmts, src->stmts, dst->num_stmts * sizeof(GpStatement));
}

void gp_program_delete(GpProgram * program)
{
	delete(program->stmts);
	delete(program);
}

// Test if two programs are _relatively_ equal
int gp_program_equal(GpProgram * a, GpProgram * b)
{
	uint i;

	if (a->num_stmts != b->num_stmts)
		return 0;

	for (i = 0; i < a->num_stmts; i++)
	{
		if (a->stmts[i].op != b->stmts[i].op)
			return 0;

		if (a->stmts[i].output != b->stmts[i].output)
			return 0;
	}

	return 1;
}

//                    Evolution Operations
// --------------------------------------------------------------

// ### Genetic Operators ####

// Mutate an individual by randomly changing some of its instructions
void gp_mutate(GpWorld * world, GpProgram * program)
{
	gp_num_t percent = rand_num();
	// prefer lower percents
	/* percent = percent * percent; */
	uint len = (uint)(percent * program->num_stmts);
	while (len--)
		program->stmts[urand(0, program->num_stmts)] = gp_random_statement(world);
}

// Two point crossover, needed for introducting length changes
void gp_cross_twopoint(GpProgram * mom, GpProgram * dad, GpProgram * child)
{
	uint mom_cp1 = urand(1, mom->num_stmts - 1);
	uint mom_cp2 = urand(1, mom->num_stmts - 1);

	uint dad_cp1 = urand(1, dad->num_stmts / 2 + 1);
	uint dad_cp2 = urand(dad_cp1, dad->num_stmts / 2 + 1);

	uint tmp;

	// If the first cross point is greater then the second, swap them
	if (mom_cp1 > mom_cp2) { tmp = mom_cp2; mom_cp2 = mom_cp1; mom_cp1 = tmp; }
	if (dad_cp1 > dad_cp2) { tmp = dad_cp2; dad_cp2 = dad_cp1; dad_cp1 = tmp; }

	child->num_stmts = mom->num_stmts - (mom_cp2 - mom_cp1) + (dad_cp2 - dad_cp1);
	child->stmts = new_array(GpStatement, child->num_stmts);

	uint i, j;

	for (i = 0; i < mom_cp1; i++)
		child->stmts[i] = mom->stmts[i];

	for (j = dad_cp1; j < dad_cp2; j++)
		child->stmts[i++] = dad->stmts[j];

	for (j = mom_cp2; j < mom->num_stmts; j++)
		child->stmts[i++] = mom->stmts[j];
}

// Homologous crossover technique that maintains lengths, from discipulus.
void gp_cross_homologous(GpProgram * mom, GpProgram * dad, GpProgram * children)
{
	uint i;
	uint max = umin(mom->num_stmts, dad->num_stmts);
	uint cp1 = urand(1, max - 1);
	uint cp2 = urand(cp1, max);

	children[0].num_stmts = mom->num_stmts;
	children[1].num_stmts = dad->num_stmts;
	children[0].stmts = new_array(GpStatement, children[0].num_stmts);
	children[1].stmts = new_array(GpStatement, children[1].num_stmts);

	GpStatement * stmts1 = children[0].stmts;
	GpStatement * stmts2 = children[1].stmts;

	for (i = 0; i < cp1; i++)
	{
		stmts1[i] = mom->stmts[i];
		stmts2[i] = dad->stmts[i];
	}

	for (; i < cp2; i++)
	{
		stmts1[i] = dad->stmts[i];
		stmts2[i] = mom->stmts[i];
	}

	for (i = cp2; i < mom->num_stmts; i++)
		stmts1[i] = mom->stmts[i];
	for (i = cp2; i < dad->num_stmts; i++)
		stmts2[i] = dad->stmts[i];

}

// ### Selection Mechanisms ###

// `roulette_select` selects over all individuals with a chance proportional
// to their fitness.
// Roulette selection might be ideal, but unfortunately it has O(n) time
// complexity and thus makes the evolve step loop O(n^2)
static inline GpProgram * roulette_select(GpWorld * world, gp_fitness_t total_fitness)
{
	gp_fitness_t targ = rand_double() * total_fitness;
	gp_fitness_t cur = 0;
	uint cur_idx = 0;

	while (cur < targ)
		cur += world->programs[cur_idx++].fitness;
	return world->programs + (cur_idx - 1);
}

// `tournament_select` uniformly selects a few individuals and returns the
// one with the highest fitness. This method is only O(1).
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

// ### Runtime & Debug ####

// Print out a program's instructions, one per line
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
				printf("r%u", stmt->args[j].data.reg);
				break;
			case GP_ARG_CONSTANT:
				printf("%f", stmt->args[j].data.num);
				break;
			case GP_ARG_INPUT:
				printf("i%u", stmt->args[j].data.reg);
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

// `gp_program_run` will execute the supplied `program` given inputs
// and return the final run state
GpState gp_program_run(GpWorld * world, GpProgram * program, gp_num_t * inputs)
{
	GpState state;
	state.ip = 0;
	memset(state.registers, 0, world->conf.num_registers * sizeof(gp_num_t));
	state.inputs = inputs;
	while (state.ip < program->num_stmts)
	{
		GpStatement * stmt = program->stmts + state.ip;
		(stmt->op->func)(&state, stmt->args, state.registers + stmt->output);
		state.ip++;
	}
	return state;
}

// Sort programs based on their fitness
static void gp_sort_programs(GpWorld * world)
{
	// This macro-style qsort avoids function calls and contains
	// performance improvements over stdlib's qsort.
#define _CMP(a,b) (a->fitness > b->fitness)
	QSORT(GpProgram, world->programs, world->conf.population_size, _CMP);
#undef _CMP
}


// `gp_world_evolve_step` runs one complete step of evolution. It
// evaluates all individuals' fitnesses, selects some, and applies
// genetic operators to create a new generation
static inline void gp_world_evolve_step(GpWorld * world)
{
	uint i;
	uint popsize = world->conf.population_size;
	gp_fitness_t total_fitness = 0;

	for (i = 0; i < popsize; i++)
	{
		world->programs[i].fitness = world->conf.evaluator(world, world->programs + i);
		total_fitness += world->programs[i].fitness;
	}

	gp_sort_programs(world);

	world->data.avg_fitness = total_fitness / (gp_fitness_t)popsize;
	world->data.best_fitness = world->programs[0].fitness;

	GpProgram new_programs[popsize];

	float cross_range = world->conf.crossover_rate;
	float mutate_range = world->conf.mutate_rate + cross_range;

	for (i = 0; i < popsize; i += 2)
	{
		float opt = rand_float();

		GpProgram * p1 = tournament_select(world);
		GpProgram * p2 = tournament_select(world);

		if (opt <= cross_range)
		{
			if (rand_float() < 0.1f)
			{
				gp_cross_twopoint(p1, p2, new_programs + i);
				gp_cross_twopoint(p1, p2, new_programs + i + 1);
			}
			else
				gp_cross_homologous(p1, p2, new_programs + i);
		}
		else if (opt <= mutate_range)
		{
			gp_program_copy(p1, new_programs + i);
			gp_program_copy(p2, new_programs + i + 1);
			gp_mutate(world, new_programs + i);
			gp_mutate(world, new_programs + i + 1);
		}
		else
		{
			gp_program_copy(p1, new_programs + i);
			gp_program_copy(p2, new_programs + i + 1);
		}
	}

	for (i = 0; i < popsize; i++)
		delete(world->programs[i].stmts);

	memcpy(world->programs, new_programs, sizeof(GpProgram) * popsize);
}

// Run `times` evolve steps
void gp_world_evolve(GpWorld * world, uint times)
{
	while (times--)
		gp_world_evolve_step(world);
}

// Run evolve steps continuously until `nsecs` seconds has passed.
// Returns the number of iterations taken.
uint gp_world_evolve_secs(GpWorld * world, uint nsecs)
{
	const clock_t nclocks = nsecs * CLOCKS_PER_SEC;
	clock_t start = clock();
	uint times = 0;

	while (clock() - start < nclocks)
	{
		gp_world_evolve_step(world);
		times++;
	}

	return times;
}
