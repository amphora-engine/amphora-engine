#include <time.h>

#include "internal/context.h"
#include "internal/random.h"

static struct random_ctx init;
static struct random_ctx *inst = &init;

int
Amphora_GetRandomV1(int n)
{
	if (n > UINT16_MAX) n = UINT16_MAX;
	if (n <= 0) return 0;

	inst->rand_state ^= inst->rand_state << 13;
	inst->rand_state ^= inst->rand_state >> 17;
	inst->rand_state ^= inst->rand_state << 5;
	return (int)((inst->rand_state >> 16) * n) >> 16;
}

float
Amphora_GetRandomFV1(void)
{
	return (float)Amphora_GetRandomV1(UINT16_MAX) / (float)UINT16_MAX;
}

/*
 * Internal functions
 */

void
Amphora_InitRand(void)
{
	init.rand_state = time(0);
}
