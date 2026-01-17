#include "internal/context.h"
#include "internal/error.h"
#include "internal/db.h"
#include "internal/lib.h"
#include "internal/memory.h"

static struct db_ctx init;
static struct db_ctx *inst = &init;

/*
 * Internal functions
 */

sqlite3 *
Amphora_GetDB(void)
{
	return inst->game_db;
}

int
Amphora_InitDB(void)
{
	char *path = Amphora_HeapStrdup(SDL_GetPrefPath(inst->game_author, inst->game_title));

	(void)Amphora_ConcatString(&path, "amphora.db");
	if (sqlite3_open_v2(path, &inst->game_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
	{
		Amphora_HeapFree(path);
		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	Amphora_HeapFree(path);

	return AMPHORA_STATUS_OK;
}

void
Amphora_CloseDB(void)
{
	(void)sqlite3_close_v2(inst->game_db);
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterGameData(const char *author, const char *title)
{
	init.game_author = author;
	init.game_title = title;
}
