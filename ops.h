
#define GP_OPERATION(name, na) \
	static const uint gp_op_##name##_nargs = na; \
	void gp_op_##name(RunState * state, program_num ** args, program_num * out)

typedef struct {
	uint nargs;
	Operation op;
} OperationList[];

#define GP_OP(name) { gp_op_##name##_nargs, &gp_op_##name }

#define GP_Out    (*out)
#define GP_Arg(x) (*(args[(x)]))

GP_OPERATION(add, 2)
{
	GP_Out = GP_Arg(0) + GP_Arg(1);
}

GP_OPERATION(mul, 2)
{
	GP_Out = GP_Arg(0) * GP_Arg(1);
}

GP_OPERATION(binnot, 1)
{
	GP_Out = ~GP_Arg(0);
}
