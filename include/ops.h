
#include <math.h>

typedef void (*GpOperationFunc)(GpState *, GpArg *, gp_num_t *);

typedef struct {
	uint num_args;
	const char * name;
	const char * infix;
	GpOperationFunc func;
} GpOperation;

#define GP_OPERATION_DECL(fn, nargs, infix_str)							\
	static void gp_op_func_##fn(GpState *, GpArg *, gp_num_t *);		\
	static const GpOperation gp_op_##fn = {								\
		.num_args = nargs,												\
		.name =  #fn,													\
		.func = &gp_op_func_##fn,										\
		.infix = infix_str												\
	};																	\
	static void gp_op_func_##fn(GpState * state, GpArg * args, gp_num_t * out)

#define GP_OPERATION(fn, nargs) GP_OPERATION_DECL(fn, nargs, NULL)
#define GP_OPERATION_INFIX(fn, i) GP_OPERATION_DECL(fn, 2, i)

#define GP_Out    (*out)
#define GP_Arg(x) 														\
	(args[x].type == GP_ARG_REGISTER ?									\
		state->registers[args[x].data.reg] :							\
		(args[x].type == GP_ARG_CONSTANT ?								\
			args[x].data.num :											\
			state->inputs[args[x].data.reg]))

#define GP_MAX_ARGS 2

GP_OPERATION(eq, 1)
{
	GP_Out = GP_Arg(0);
}

GP_OPERATION_INFIX(add, "+")
{
	GP_Out = GP_Arg(0) + GP_Arg(1);
}

GP_OPERATION_INFIX(sub, "-")
{
	GP_Out = GP_Arg(0) - GP_Arg(1);
}

GP_OPERATION_INFIX(mul, "*")
{
	GP_Out = GP_Arg(0) * GP_Arg(1);
}

GP_OPERATION_INFIX(div, "/")
{
	const gp_num_t arg1 = GP_Arg(1);
	if (arg1 == 0)
		GP_Out = 0;
	else
		GP_Out = GP_Arg(0) / arg1;
}

GP_OPERATION(square, 1)
{
	GP_Out = GP_Arg(0) * GP_Arg(0);
}

GP_OPERATION(abs, 1)
{
	GP_Out = fabs(GP_Arg(0));
}

GP_OPERATION(pow, 2)
{
	GP_Out = pow(GP_Arg(0), fmod(GP_Arg(1), 10.0));
}

// BITWISE FUNCTIONS

GP_OPERATION(binnot, 1)
{
	GP_Out = (gp_num_t)(~((uint)GP_Arg(0)));
}

GP_OPERATION(xor, 2)
{
	GP_Out = (gp_num_t)((uint)GP_Arg(0) ^ (uint)GP_Arg(1));
}
