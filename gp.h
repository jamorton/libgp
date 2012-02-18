
#ifndef __GP_H__
#define __GP_H__

#define NUM_REGISTERS 2
#define MIN_LENGTH 1
#define MAX_LENGTH 5

typedef unsigned int program_num;
typedef unsigned int uint;

typedef struct {
	program_num registers[NUM_REGISTERS];
	uint ip;
} RunState;

typedef void (*Operation)(RunState *, program_num **, program_num *);
		
typedef struct {
	program_num * args[NUM_REGISTERS];
	Operation op;
} Statement;

typedef struct {
	uint num_stmts;
	Statement * stmts;
} Program;


#endif
