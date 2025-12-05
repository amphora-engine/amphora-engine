#include "internal/db.h"
#include "internal/memory.h"
#include "internal/save_data.h"

#include "SDL.h"

int
Amphora_SaveNumber(const char *attribute, double value)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "INSERT OR REPLACE INTO save_data (attribute, value) VALUES (?, ?);";

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, attribute, -1, NULL);
	sqlite3_bind_double(stmt, 2, value);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

int
Amphora_SaveString(const char *attribute, const char *value)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "INSERT OR REPLACE INTO save_data (attribute, value) VALUES (?, ?);";

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, attribute, -1, NULL);
	sqlite3_bind_text(stmt, 2, value, -1, NULL);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return 0;
}

double
Amphora_LoadNumber(const char *attribute, double default_value)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "SELECT value FROM save_data WHERE attribute=?";
	double val;

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, attribute, -1, NULL);
	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return default_value;
	}
	val = sqlite3_column_double(stmt, 0);
	sqlite3_finalize(stmt);

	return val;
}

int
Amphora_LoadString(const char *attribute, char **out_string)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "SELECT value FROM save_data WHERE attribute=?";

	sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, attribute, -1, NULL);
	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return -1;
	}
	*out_string = Amphora_HeapStrdupFrame((char *)sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);

	return 0;
}

/*
 * Internal functions
 */

int
Amphora_InitSave(void)
{
	sqlite3 *db = Amphora_GetDB();
	const char *sql = "CREATE TABLE IF NOT EXISTS save_data("
			  "attribute TEXT PRIMARY KEY NOT NULL,"
			  "value ANY);";
	char *err_msg;
	sqlite3_exec(db, sql, NULL, NULL, &err_msg);
	if (err_msg)
	{
		SDL_Log("%s\n", err_msg);
		return -1;
	}

	return 0;
}
