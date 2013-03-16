
#include <math.h>

typedef void (*GpOperationFunc)(GpState *, GpArg *, gp_num_t *);

#define GP_OPERATION(name, na) \
	static const uint gp_op_##name##_nargs = na; \
	static const char * gp_op_##name##_name = #name; \
	static void gp_op_##name(GpState * state, GpArg * args, gp_num_t * out)

#define GP_Out    (*out)
#define GP_Arg(x) (gp_get_arg(state, args[x]))

typedef struct {
	uint num_args;
	const char * name;
	GpOperationFunc func;
} GpOperation;

#define GP_OP(name) ((GpOperation){ gp_op_##name##_nargs, gp_op_##name##_name, &gp_op_##name })

#define GP_MAX_ARGS 2

static gp_num_t gp_get_arg(GpState * state, GpArg arg)
{
	if (arg.type == GP_ARG_REGISTER)
		return state->registers[arg.data.reg];
	else if (arg.type == GP_ARG_CONSTANT)
		return arg.data.num;
	return state->inputs[arg.data.reg];
}

GP_OPERATION(eq, 1)
{
	GP_Out = GP_Arg(0);
}

GP_OPERATION(add, 2)
{
	GP_Out = GP_Arg(0) + GP_Arg(1);
}

GP_OPERATION(sub, 2)
{
	GP_Out = GP_Arg(0) - GP_Arg(1);
}

GP_OPERATION(mul, 2)
{
	GP_Out = GP_Arg(0) * GP_Arg(1);
}

GP_OPERATION(div, 2)
{
	if (GP_Arg(1) == 0)
		GP_Out = 0;
	else
		GP_Out = GP_Arg(0) / GP_Arg(1);
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
	GP_Out = pow(GP_Arg(0), (int)GP_Arg(1) % 5);
	/*	GP_Out = pow(GP_Arg(0), fmod(GP_Arg(1), 10.0)); */
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
