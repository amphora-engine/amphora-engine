#include "internal/context.h"
#include "internal/ht_hash.h"
#include "internal/session_data.h"

static struct session_data_ctx init;
static struct session_data_ctx *inst = &init;

long
Amphora_GetSessionDataV1(const char *key)
{
	long d;

#ifdef DEBUG
	SDL_Log("Read %ld from session data key %s\n", d = HT_GetValue(key, inst->sd), key);
#endif
	return d;
}

void
Amphora_StoreSessionDataV1(const char *key, long val)
{
#ifdef DEBUG
	SDL_Log("Storing %ld in session data with key %s\n", val, key);
#endif
	HT_SetValue(key, val, inst->sd);
}

void
Amphora_DeleteSessionDataV1(const char *key)
{
#ifdef DEBUG
	SDL_Log("Deleting key %s from session data\n", key);
#endif
	HT_DeleteKey(key, inst->sd);
}

/*
 * Internal functions
 */

void
Amphora_InitSessionData(void)
{
	inst->sd = HT_NewTable();
}

void
Amphora_DeInitSessionData(void)
{
	HT_FreeTable(inst->sd);
}
