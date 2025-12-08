#include "internal/random.h"

#include <time.h>

static Uint32 rand_state;

int
Amphora_GetRandomV1(int n)
{
	if (n > UINT16_MAX) n = UINT16_MAX;
	if (n <= 0) return 0;

	rand_state ^= rand_state << 13;
	rand_state ^= rand_state >> 17;
	rand_state ^= rand_state << 5;
	return (int)((rand_state >> 16) * n) >> 16;
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
	rand_state = time(0);
}
