
#include "gp.h"
#include "mem.h"
#include "iqsort.h"

#include <time.h>
#include <string.h>

sfmt_t _sfmt;
static int _sfmt_has_init = 0;

// `gp_world_new` creates a new world with a default config.
// After a world is created, custom configuration should be set and
// the desired list of operations should be added with `gp_world_add_op`.
GpWorld * gp_world_new()
{
	if (!_sfmt_has_init) {
		sfmt_init_gen_rand(&_sfmt, time(NULL));
		_sfmt_has_init = 1;
	}

	GpWorld * world = new(GpWorld);
	world->programs = NULL;

	world->stats.total_steps = 0;
	world->stats.avg_fitness = 0;
	world->stats.best_fitness = 0;

	world->_stmt_buf = NULL;
	world->_last_optimize = 0;

	return world;
}

void gp_world_delete(GpWorld * world)
{
	delete(world->programs);
	delete(world->_stmt_buf);
	delete(world);
}

static GpOperation _default_ops[5];

// Returns a default (sane) config
GpWorldConf gp_world_conf_default()
{
	// bah.
	_default_ops[0] = gp_op_add;
	_default_ops[1] = gp_op_sub;
	_default_ops[2] = gp_op_mul;
	_default_ops[3] = gp_op_div;
	_default_ops[4] = gp_op_eq;

	return (GpWorldConf) {
		.ops = _default_ops,
		.num_ops = 5,
		.evaluator = NULL,
		.constant_func = NULL,
		.population_size = 50000,
		.num_inputs = 0,
		.num_registers = 2,
		.min_program_length = 5,
		.max_program_length = 30,
		.mutate_rate = 0.4,
		.crossover_rate = 0.9,
		.homologous_rate = 0.9,
		.minimize_fitness = 0,
		.auto_optimize = 1
	};
}

static void _init_err(const char * estr)
{
	printf("libgp init ERROR: %s\n", estr);
	abort();
}

// `gp_world_initialize` should be called after the world has been
// created and completely configured. No configuration should be changed
// after calling this.
void gp_world_initialize(GpWorld * world, GpWorldConf conf)
{
	if (conf.constant_func == NULL || conf.evaluator == NULL)
		_init_err("constant_func or evaluator not defined");

	if (conf.num_registers > GP_MAX_REGISTERS)
		_init_err("num_registers is greater than GP_MAX_REGISTERS");

	if (conf.min_program_length < 3)
		_init_err("min_program_length must be 3 or greater");

	if (conf.max_program_length < world->conf.min_program_length)
		_init_err("max_program_length cannot be greater than min_program_length");

	if ((conf.population_size & 1) != 0)
		_init_err("population size must be an even number");

	if (conf.num_inputs > conf.num_registers)
		_init_err("num_inputs cannot be greater than num_registers");

	world->conf = conf;
	world->has_init = 1;

	world->programs = new_array(GpProgram, world->conf.population_size);

	int bufsize = conf.population_size * conf.max_program_length;
	world->_stmt_buf = new_array(GpStatement, bufsize);

	uint i, j;
	for (i = 0; i < world->conf.population_size; i++) {
		GpProgram * program = world->programs + i;
		program->evaluated = 0;
		program->stmts = world->_stmt_buf + i * conf.max_program_length;
		program->num_stmts = urand(world->conf.min_program_length,
			world->conf.max_program_length + 1);
		for (j = 0; j < program->num_stmts; j++)
			program->stmts[j] = gp_statement_random(world);
	}

	if (world->conf.auto_optimize)
		gp_world_optimize(world);

	for (i = 0; i < world->conf.population_size; i++) {
		world->programs[i].fitness = world->conf.evaluator(world, world->programs + i);
		world->programs[i].evaluated = 1;
	}
}

// Mutate an individual by randomly changing some of its instructions
void gp_mutate(GpWorld * world, GpProgram * program)
{
	program->stmts[urand(0, program->num_stmts)] = gp_statement_random(world);
}

// Two point crossover, needed for introducting length changes
void gp_cross_twopoint(GpWorld * world, GpProgram * mom, GpProgram * dad, GpProgram * child)
{
	uint mom_cp1 = urand(1, mom->num_stmts);
	uint mom_cp2 = urand(1, mom->num_stmts);

	//uint dad_cp1 = urand(1, dad->num_stmts / 2 + 1);
	//uint dad_cp2 = urand(dad_cp1, dad->num_stmts / 2 + 1);
	uint dad_cp1 = urand(1, dad->num_stmts);
	uint dad_cp2 = urand(1, dad->num_stmts);

	uint tmp;

	// If the first cross point is greater then the second, swap them
	if (mom_cp1 > mom_cp2) { tmp = mom_cp2; mom_cp2 = mom_cp1; mom_cp1 = tmp; }
	if (dad_cp1 > dad_cp2) { tmp = dad_cp2; dad_cp2 = dad_cp1; dad_cp1 = tmp; }

	uint stmts = mom->num_stmts - (mom_cp2 - mom_cp1) + (dad_cp2 - dad_cp1);
	const uint max_len = world->conf.max_program_length;
	const uint min_len = world->conf.min_program_length;

	if (stmts > max_len)
	{
		while (stmts > max_len && dad_cp2 > dad_cp1 + 1) {
			dad_cp2--;
			stmts--;
		}
		while (stmts > max_len && mom_cp2 < mom->num_stmts - 1) {
			mom_cp2++;
			stmts--;
		}
		while (stmts > max_len && mom_cp1 > 1) {
			mom_cp1--;
			stmts--;
		}
	}

	if (stmts < min_len)
	{
		while (stmts < min_len && dad_cp1 > 0) {
			dad_cp1--;
			stmts++;
		}
		while (stmts < min_len && dad_cp2 < dad->num_stmts) {
			dad_cp2++;
			stmts++;
		}
	}

	child->num_stmts = stmts;

	uint i, j;

	for (i = 0; i < mom_cp1; i++)
		child->stmts[i] = mom->stmts[i];

	for (j = dad_cp1; j < dad_cp2; j++)
		child->stmts[i++] = dad->stmts[j];

	for (j = mom_cp2; j < mom->num_stmts; j++)
		child->stmts[i++] = mom->stmts[j];
}

// Homologous crossover technique that maintains lengths, from discipulus.
void gp_cross_homologous(GpProgram * mom, GpProgram * dad, GpProgram * c1, GpProgram * c2)
{
	uint i;
	uint max = umin(mom->num_stmts, dad->num_stmts);
	uint cp1 = urand(1, max - 1);
	uint cp2 = urand(cp1, max);

	c1->num_stmts = mom->num_stmts;
	c2->num_stmts = dad->num_stmts;

	GpStatement * stmts1 = c1->stmts;
	GpStatement * stmts2 = c2->stmts;

	for (i = 0; i < cp1; i++) {
		stmts1[i] = mom->stmts[i];
		stmts2[i] = dad->stmts[i];
	}

	for (; i < cp2; i++) {
		stmts1[i] = dad->stmts[i];
		stmts2[i] = mom->stmts[i];
	}

	for (i = cp2; i < mom->num_stmts; i++)
		stmts1[i] = mom->stmts[i];
	for (i = cp2; i < dad->num_stmts; i++)
		stmts2[i] = dad->stmts[i];
}

// `gp_world_evolve_steady_state` uses a steady-state evolutionary algorithm
// that will only perform one "breeding" operation per step
// Each call will replace two programs with new ones
static void gp_world_evolve_steady_state(GpWorld * world)
{
	const uint popsize = world->conf.population_size;

	// selection by tournament: we pick 4 programs, the best two are mated and the offspring
	// replace the worst 2.
	GpProgram * progs[] = {
		world->programs + urand(0, popsize),
		world->programs + urand(0, popsize),
		world->programs + urand(0, popsize),
		world->programs + urand(0, popsize)
	};

	// Fast sort for 4 elements.
	// Note: calling min and max to swap elements may seem inefficient, but
	// gcc will detect this sequence and use extremely fast conditional move
	// instructions. If we try to optimize this, it will become much slower.
	// see: http://stackoverflow.com/questions/2786899/

#define min(x,y) (x->fitness < y->fitness ? x : y)
#define max(x,y) (x->fitness < y->fitness ? y : x)

	if (world->conf.minimize_fitness) {
#define SWAP(x,y) {	GpProgram * tmp = min(progs[x], progs[y]); progs[y] = max(progs[x], progs[y]); progs[x] = tmp; }
	SWAP(0, 1);
	SWAP(2, 3);
	SWAP(0, 2);
	SWAP(1, 3);
	SWAP(1, 2);
#undef SWAP
	} else {
#define SWAP(x,y) { GpProgram * tmp = max(progs[x], progs[y]); progs[y] = min(progs[x], progs[y]); progs[x] = tmp; }
	SWAP(0, 1);
	SWAP(2, 3);
	SWAP(0, 2);
	SWAP(1, 3);
	SWAP(1, 2);
#undef SWAP
	}

#undef max
#undef min

	world->stats.total_steps++;

	if (progs[2] == progs[3])
		return;

	if (rand_double() < world->conf.crossover_rate)
	{
		if (rand_double() < world->conf.homologous_rate)
			gp_cross_homologous(progs[0], progs[1], progs[2], progs[3]);
		else
		{
			gp_cross_twopoint(world, progs[0], progs[1], progs[2]);
			gp_cross_twopoint(world, progs[0], progs[1], progs[3]);
		}
	}
	else
	{
		gp_program_copy(progs[0], progs[2]);
		gp_program_copy(progs[1], progs[3]);
	}

	if (rand_double() < world->conf.mutate_rate)
		gp_mutate(world, progs[2]);
	if (rand_double() < world->conf.mutate_rate)
		gp_mutate(world, progs[3]);

	progs[2]->fitness = world->conf.evaluator(world, progs[2]);
	progs[3]->fitness = world->conf.evaluator(world, progs[3]);

	if (world->conf.auto_optimize && world->stats.total_steps % 300000 == 0)
		gp_world_optimize(world);
}

// Sort programs based on their fitness
static void _sort_programs(GpWorld * world)
{
	// This macro-style qsort avoids function calls and contains
	// performance improvements over stdlib's qsort.
#define CMP_L(a, b) (a->fitness < b->fitness)
#define CMP_G(a, b) (a->fitness > b->fitness)

	if (world->conf.minimize_fitness)
	{
		QSORT(GpProgram, world->programs, world->conf.population_size, CMP_L);
	}
	else
	{
		QSORT(GpProgram, world->programs, world->conf.population_size, CMP_G);
	}

#undef CMP_G
#undef CMP_L
}

//
// Recalculates various statistics in world->stats and sorts
// the program by their fitness in descending order
//
static void _process_stats(GpWorld * world)
{
	gp_fitness_t total_fitness = 0.0;
	int total_length = 0;

	for (uint i = 0; i < world->conf.population_size; i++) {
		total_fitness += world->programs[i].fitness;
		total_length  += world->programs[i].num_stmts;
	}

	_sort_programs(world);

	world->stats.avg_fitness = total_fitness / (gp_fitness_t)world->conf.population_size;
	world->stats.best_fitness = world->programs[0].fitness;
	world->stats.total_generations = world->stats.total_steps * 2 / world->conf.population_size;
	world->stats.avg_program_length = total_length / (float)world->conf.population_size;
}

// Evolve `times` steps
void gp_world_evolve_times(GpWorld * world, uint times)
{
	while (times--)
		gp_world_evolve_steady_state(world);
	_process_stats(world);
}

// Evolve until `gens` generations have passed.
void gp_world_evolve_gens(GpWorld * world, uint gens)
{
	const uint times = gens * world->conf.population_size / 2;
	gp_world_evolve_times(world, times);
}

// Run evolve steps continuously until `nsecs` seconds have passed.
// Returns the number of iterations taken.
uint gp_world_evolve_secs(GpWorld * world, float nsecs)
{
	const clock_t nclocks = (clock_t)(nsecs * CLOCKS_PER_SEC);
	clock_t start = clock();
	uint times = 0;

	while (clock() - start < nclocks)
	{
		gp_world_evolve_steady_state(world);
		times++;
	}

	_process_stats(world);

	return times;
}
