
#include "cgp.h"
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
	world->num_ops = 0;
	world->ops = NULL;
	world->programs = NULL;

	world->stats.total_steps = 0;
	world->stats.avg_fitness = 0;
	world->stats.best_fitness = 0;

	world->_stmt_buf = NULL;

	return world;
}

GpWorldConf gp_world_conf_default()
{
	GpWorldConf conf;
	conf.population_size    = 10000;
	conf.num_registers      = 2;
	conf.num_inputs         = 0;
	conf.min_program_length = 1;
	conf.max_program_length = 10;
	conf.mutate_rate        = 0.40;
	conf.crossover_rate     = 0.90;
	conf.evaluator          = NULL;
	conf.constant_func      = NULL;
	conf.minimize_fitness   = 0;
	return conf;
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
}

// `gp_world_add_op` is used to  make an operation available for
// use in programs. For example:
//
//     gp_world_add_op(world, gp_op_add);
//     gp_world_add_op(world, gp_op_sub);
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

// Mutate an individual by randomly changing some of its instructions
void gp_mutate(GpWorld * world, GpProgram * program)
{
	double opt = rand_double();
	uint i, idx;

	// Change a statement
	if (opt < 0.33) {
		program->stmts[urand(0, program->num_stmts)] = gp_statement_random(world);

		// Insert a random statement
	} else if (opt < 0.66 && program->num_stmts < world->conf.max_program_length) {
		program->num_stmts++;
		idx = urand(0, program->num_stmts);
		for (i = program->num_stmts - 1; i > idx; i--)
			program->stmts[i] = program->stmts[i - 1];
		program->stmts[idx] = gp_statement_random(world);

		// Delete a random statement
	} else if (program->num_stmts > world->conf.min_program_length) {
		idx = urand(0, program->num_stmts);
		program->num_stmts--;
		for (i = idx; i < program->num_stmts; i++)
			program->stmts[i] = program->stmts[i + 1];
	}
}

// Two point crossover, needed for introducting length changes
void gp_cross_twopoint(GpWorld * world, GpProgram * mom, GpProgram * dad, GpProgram * child)
{
	uint mom_cp1 = urand(1, mom->num_stmts - 1);
	uint mom_cp2 = urand(1, mom->num_stmts - 1);

	//uint dad_cp1 = urand(1, dad->num_stmts / 2 + 1);
	//uint dad_cp2 = urand(dad_cp1, dad->num_stmts / 2 + 1);
	uint dad_cp1 = urand(1, dad->num_stmts - 1);
	uint dad_cp2 = urand(1, dad->num_stmts - 1);

	uint tmp;

	// If the first cross point is greater then the second, swap them
	if (mom_cp1 > mom_cp2) { tmp = mom_cp2; mom_cp2 = mom_cp1; mom_cp1 = tmp; }
	if (dad_cp1 > dad_cp2) { tmp = dad_cp2; dad_cp2 = dad_cp1; dad_cp1 = tmp; }

	uint stmts = mom->num_stmts - (mom_cp2 - mom_cp1) + (dad_cp2 - dad_cp1);
	uint max_len = world->conf.max_program_length;
	if (stmts > world->conf.max_program_length) {
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

	child->num_stmts = stmts;
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

// Sort programs based on their fitness
static void gp_sort_programs(GpWorld * world)
{
	// This macro-style qsort avoids function calls and contains
	// performance improvements over stdlib's qsort.
	if (world->conf.minimize_fitness)
	{
        #define CMP_L(a, b) (a->fitness < b->fitness)
		QSORT(GpProgram, world->programs, world->conf.population_size, CMP_L);
		#undef CMP_L
	}
	else
	{
        #define CMP_G(a, b) (a->fitness > b->fitness)
		QSORT(GpProgram, world->programs, world->conf.population_size, CMP_G);
		#undef CMP_G
	}
}

// `gp_world_evolve_steady_state` uses a steady-state evolutionary algorithm
// that will only perform one "breeding" operation per step
static void gp_world_evolve_steady_state(GpWorld * world)
{
	const uint popsize = world->conf.population_size;
	uint i;

	if (world->stats.total_steps == 0) {
		for (i = 0; i < popsize; i++) {
			world->programs[i].fitness = world->conf.evaluator(world, world->programs + i);
			world->programs[i].evaluated = 1;
		}
	}

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
		gp_cross_homologous(progs[0], progs[1], progs[2], progs[3]);
	else {
		gp_program_copy(progs[0], progs[2]);
		gp_program_copy(progs[1], progs[3]);
	}

	if (rand_double() < world->conf.mutate_rate) {
		gp_mutate(world, progs[2]);
		gp_mutate(world, progs[3]);
	}

	progs[2]->fitness = world->conf.evaluator(world, progs[2]);
	progs[3]->fitness = world->conf.evaluator(world, progs[3]);
}

static void _process_stats(GpWorld * world)
{
	gp_fitness_t tot = 0.0;
	for (uint i = 0; i < world->conf.population_size; i++)
		tot += world->programs[i].fitness;

	gp_sort_programs(world);
	world->stats.avg_fitness = tot / (gp_fitness_t)world->conf.population_size;
	world->stats.best_fitness = world->programs[0].fitness;
}

// Run `times` evolve steps
void gp_world_evolve(GpWorld * world, uint times)
{
	while (times--)
		gp_world_evolve_steady_state(world);
}

// Run evolve steps continuously until `nsecs` seconds has passed.
// Returns the number of iterations taken.
uint gp_world_evolve_secs(GpWorld * world, uint nsecs)
{
	const clock_t nclocks = nsecs * CLOCKS_PER_SEC;
	clock_t start = clock();
	uint times = 0;

	while (clock() - start < nclocks) {
		gp_world_evolve_steady_state(world);
		times++;
	}

	_process_stats(world);

	return times;
}
