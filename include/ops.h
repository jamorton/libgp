
#define GP_OPERATION(name, na) \
	static const uint gp_op_##name##_nargs = na; \
	static const char * gp_op_##name##_name = #name; \
	void gp_op_##name(GpState * state, gp_num ** args, gp_num * out)

#define GP_Out    (*out)
#define GP_Arg(x) (*(args[(x)]))

typedef struct {
	uint num_args;
	const char * name;
	GpOperationFunc func;
} GpOperation;

#define GP_OP(name) ((GpOperation){ gp_op_##name##_nargs, gp_op_##name##_name, &gp_op_##name })

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
