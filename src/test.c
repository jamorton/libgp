

#include "gp.h"

static int primes[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,1163,1171,1181,1187,1193,1201,1213,1217,1223,1229,1231,1237,1249,1259,1277,1279,1283,1289,1291,1297,1301,1303,1307,1319,1321,1327,1361,1367,1373,1381,1399,1409,1423,1427,1429,1433,1439,1447,1451,1453,1459,1471,1481,1483,1487,1489,1493,1499,1511,1523,1531,1543,1549,1553,1559,1567,1571,1579,1583,1597,1601,1607,1609,1613,1619,1621,1627,1637,1657,1663,1667,1669,1693,1697,1699,1709,1721,1723,1733,1741,1747,1753,1759,1777,1783,1787,1789,1801,1811,1823,1831,1847,1861,1867,1871,1873,1877,1879,1889,1901,1907,1913,1931,1933,1949,1951,1973,1979,1987,1993,1997,1999,2003,2011,2017,2027,2029,2039,2053,2063,2069,2081,2083,2087,2089,2099,2111,2113,2129,2131,2137,2141,2143,2153,2161,2179,2203,2207,2213,2221,2237,2239,2243,2251,2267,2269,2273,2281,2287,2293,2297,2309,2311,2333,2339,2341,2347,2351,2357};

static gp_fitness_t eval(GpWorld * world, GpProgram * program)
{
	uint i;
	gp_fitness_t fit = 0;
	uint consec = 0;
	for (i = 0; i < 30; i++)
	{
		gp_num_t inp = (gp_num_t)i;
		GpState state = gp_program_run(world, program, &inp);
		int out = (int)fabs(state.registers[0]);

		if (out == primes[i])
			fit += 1 + 0.25 * consec++;
		else
			consec = 0;

	}
	return fit;
}

static const gp_num_t constants[] = {
	-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,
	0,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	50,
	100
};

static gp_num_t constant_func(void)
{
	return (gp_num_t)constants[urand(0,sizeof(constants)/sizeof(gp_num_t))];
}

int main(void)
{
	GpWorld * world = gp_world_new();

	gp_world_add_op(world, GP_OP(add));
	gp_world_add_op(world, GP_OP(mul));
	gp_world_add_op(world, GP_OP(sub));
	gp_world_add_op(world, GP_OP(div));
	gp_world_add_op(world, GP_OP(eq));
	gp_world_add_op(world, GP_OP(pow));

	world->conf.constant_func      = &constant_func;
	world->conf.evaluator          = &eval;
	world->conf.population_size    = 10000;
	world->conf.num_inputs         = 1;
	world->conf.min_program_length = 4;
	world->conf.max_program_length = 20;
	world->conf.num_registers      = 2;

	gp_world_initialize(world);

	uint loops = 0;
	uint total_steps = 0;
	float ips = 0.0;
	for (;;)
	{
		uint times = gp_world_evolve_secs(world, 1);
		ips = (ips * loops + times) / (loops + 1);
		total_steps += times;
		printf("Best: %-7.2f  Avg: %-7.2f  Steps: %-6u  Steps/sec: %5.2f\n",
			   world->data.best_fitness,
			   world->data.avg_fitness,
			   total_steps,
			   ips);
		loops++;

	}
}

