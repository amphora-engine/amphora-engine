#ifdef WIN32
#include <objbase.h>
#elif __APPLE__
#include <CoreFoundation/CFUUID.h>
#else
#include "uuid/uuid.h"
#endif

#include "internal/context.h"
#include "internal/db.h"
#include "internal/lib.h"
#include "internal/memory.h"
#include "internal/prefs.h"

/* Prototypes for private functions */
static SDL_GUID Amphora_GetUUID(void);

static struct prefs_ctx init;
static struct prefs_ctx *inst = &init;

/*
 * Internal functions
 */

int
Amphora_InitConfig(void)
{
	sqlite3 *db = Amphora_GetDB();
	const char *sql = "CREATE TABLE IF NOT EXISTS prefs("
			  "uuid TEXT PRIMARY KEY NOT NULL,"
			  "win_x INT,"
			  "win_y INT,"
			  "win_flags INT,"
			  "framerate INT);";
	const char *sql_create_row = "INSERT INTO prefs (uuid) VALUES (?);";
	sqlite3_stmt *stmt;
	char *err_msg;

	sqlite3_exec(db, sql, NULL, NULL, &err_msg);
	if (err_msg)
	{
		SDL_Log("%s\n", err_msg);
		return -1;
	}
	SDL_GUIDToString(Amphora_GetUUID(), init.uuid, sizeof(init.uuid));
	sqlite3_prepare_v2(db, sql_create_row, (int)SDL_strlen(sql_create_row), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, init.uuid, -1, NULL);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

int
Amphora_SaveWinX(int win_x)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "UPDATE prefs SET win_x=? WHERE uuid=?;";

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_int64(stmt, 1, win_x);
	sqlite3_bind_text(stmt, 2, inst->uuid, -1, NULL);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

int
Amphora_SaveWinY(int win_y)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "UPDATE prefs SET win_y=? WHERE uuid=?;";

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_int64(stmt, 1, win_y);
	sqlite3_bind_text(stmt, 2, inst->uuid, -1, NULL);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

int
Amphora_SaveWinFlags(Uint32 win_flags)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "UPDATE prefs SET win_flags=? WHERE uuid=?;";

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_int64(stmt, 1, (Sint64)win_flags);
	sqlite3_bind_text(stmt, 2, inst->uuid, -1, NULL);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

int
Amphora_SaveFPS(Uint32 fps)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "UPDATE prefs SET framerate=? WHERE uuid=?;";

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_int64(stmt, 1, fps);
	sqlite3_bind_text(stmt, 2, inst->uuid, -1, NULL);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

Sint32
Amphora_LoadWinX(void)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "SELECT win_x FROM prefs WHERE uuid=?";
	Sint32 val;

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, inst->uuid, -1, NULL);
	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return inst->window_x;
	}
	val = (Sint32)sqlite3_column_int64(stmt, 0);
	sqlite3_finalize(stmt);

	return val ? val : inst->window_x;
}

Sint32
Amphora_LoadWinY(void)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "SELECT win_y FROM prefs WHERE uuid=?";
	Sint32 val;

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, inst->uuid, -1, NULL);
	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return inst->window_y;
	}
	val = (Sint32)sqlite3_column_int64(stmt, 0);
	sqlite3_finalize(stmt);

	return val ? val : inst->window_y;
}

Uint32
Amphora_LoadWinFlags(void)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "SELECT win_flags FROM prefs WHERE uuid=?";
	Sint32 val;

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, inst->uuid, -1, NULL);
	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return inst->window_flags;
	}
	val = (Sint32)sqlite3_column_int64(stmt, 0);
	sqlite3_finalize(stmt);

	return val ? (Uint32)val : inst->window_flags;
}

Sint32
Amphora_LoadFPS(void)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "SELECT framerate FROM prefs WHERE uuid=?";
	Sint32 val;

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, inst->uuid, -1, NULL);
	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return inst->framerate;
	}
	val = (Sint32)sqlite3_column_int64(stmt, 0);
	sqlite3_finalize(stmt);

	return val ? val : inst->framerate;
}

/*
 * Private functions
 */

static SDL_GUID
Amphora_GetUUID(void)
{
#ifdef _WIN32
	GUID uuid_bytes;
#elif __APPLE__
	CFUUIDRef uuid_ref;
	CFUUIDBytes uuid_bytes;
#endif
	char *path = Amphora_HeapStrdup(SDL_GetPrefPath(inst->game_author, inst->game_title));
	SDL_RWops *rw;
	char *file_contents;
	SDL_GUID guid;
	char guid_str[33];

	Amphora_ConcatString(&path, "uuid");
	if ((rw = SDL_RWFromFile(path, "rb")))
	{
#ifdef DEBUG
		SDL_Log("Loading UUID from file...\n");
#endif
		if (!((file_contents = Amphora_HeapAlloc(SDL_RWsize(rw), MEM_STRING))))
		{
			Amphora_HeapFree(path);
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate space for UUID!\n");
			SDL_memset(&guid, 0, sizeof(guid));

			return guid;
		}
		SDL_RWread(rw, file_contents, SDL_RWsize(rw), 1);
		SDL_memcpy(&guid.data, file_contents, sizeof(guid.data));
		Amphora_HeapFree(file_contents);
		SDL_RWclose(rw);

#ifdef DEBUG
		SDL_GUIDToString(guid, guid_str, sizeof(guid_str));
		SDL_Log("UUID: %s\n", guid_str);
#endif

		return guid;
	}

#ifdef DEBUG
	SDL_Log("Generating new UUID...\n");
#endif
#ifdef _WIN32
	CoCreateGuid(&uuid_bytes);
	SDL_memcpy(&guid.data[0], &uuid_bytes.Data1, sizeof(uuid_bytes.Data1));
	SDL_memcpy(&guid.data[4], &uuid_bytes.Data2, sizeof(uuid_bytes.Data2));
	SDL_memcpy(&guid.data[6], &uuid_bytes.Data3, sizeof(uuid_bytes.Data3));
	SDL_memcpy(&guid.data[8], &uuid_bytes.Data4, sizeof(uuid_bytes.Data4));
#elif __APPLE__
	uuid_ref = CFUUIDCreate(NULL);
	uuid_bytes = CFUUIDGetUUIDBytes(uuid_ref);
	CFRelease(uuid_ref);

	SDL_memcpy(&guid.data, &uuid_bytes, sizeof(guid.data));
#else
	uuid_generate(guid.data);
#endif

	rw = SDL_RWFromFile(path, "w+b");
	SDL_RWwrite(rw, &guid.data, sizeof(guid.data), 1);
	SDL_RWclose(rw);
	Amphora_HeapFree(path);

#ifdef DEBUG
	SDL_GUIDToString(guid, guid_str, sizeof(guid_str));
	SDL_Log("UUID: %s\n", guid_str);
#endif

	return guid;
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterPrefs(const char *auth, const char *title, int win_x, int win_y, unsigned int win_flags, int fr)
{
	init.game_author = auth;
	init.game_title = title;
	init.window_x = win_x;
	init.window_y = win_y;
	init.window_flags = win_flags;
	init.framerate = fr;
}
