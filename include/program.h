
#ifndef GP_INCLUDE_PROGRAM_H
#define GP_INCLUDE_PROGRAM_H

//
// ### Structures ###
//

typedef struct {
	gp_num_t registers[GP_MAX_REGISTERS];
	gp_num_t * inputs;
	uint ip;
	void * data;
} GpState;

typedef enum {
	GP_ARG_REGISTER = 0,
	GP_ARG_CONSTANT,
	GP_ARG_INPUT,
	GP_ARG_COUNT
} GpArgType;

typedef struct {
	GpArgType type;
	union {
		uint reg;
		gp_num_t num;
	} data;
} GpArg;

// This needs to be included exactly here
#include "ops.h"

typedef struct {
	uint output;
	GpArg args[GP_MAX_ARGS];
	GpOperation * op;
} GpStatement;

typedef struct {
	gp_fitness_t fitness;
	int evaluated;
	uint num_stmts;
	GpStatement * stmts;
} GpProgram;

//
// ### Function prototypes ###
//

struct GpWorld;

GpStatement gp_statement_random (struct GpWorld *);

GpProgram * gp_program_new      (struct GpWorld *);
void        gp_program_init     (struct GpWorld *, GpProgram *);
void        gp_program_copy     (GpProgram *, GpProgram *);
void        gp_program_delete   (GpProgram *);
int         gp_program_equal    (GpProgram *, GpProgram *);
GpState     gp_program_run      (struct GpWorld *, GpProgram *, gp_num_t *);
void        gp_program_print    (GpProgram *, FILE * f);

#endif
