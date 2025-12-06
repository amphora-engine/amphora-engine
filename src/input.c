#include "internal/db.h"
#include "internal/error.h"
#include "internal/img.h"
#include "internal/input.h"
#include "internal/lib.h"
#include "internal/memory.h"
#include "internal/render.h"
#include "internal/ttf.h"

/* Prototypes for private functions */
static void Amphora_ProcessJoystickState(SDL_GameControllerAxis ax, SDL_GameControllerAxis ay, Vector2f *js, bool *jactive);

/* File-scoped variables */
static Uint32 *key_actions;
static SDL_GameController *controller;
static const char **action_names;
static int action_count;
static SDL_Keycode *keys;
static SDL_GameControllerButton *controller_buttons;
static const char **controller_button_names;
static SDL_Keycode pressed_key;
static bool joystickl_active;
static bool joystickr_active;
static Vector2f joystickl_state;
static Vector2f joystickr_state;

void
Amphora_LoadKeymap(void)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql_write = "INSERT OR IGNORE INTO key_map (idx, action, key, key_name, gamepad, gamepad_name)"
				"VALUES (?, ?, ?, ?, ?, ?)";
	const char *sql_read = "SELECT key, gamepad FROM key_map ORDER BY idx";
	int sql_write_len = (int)SDL_strlen(sql_write);
	int i;

	/* Load the default keymap for any mappings that are missing */
	for (i = 0; i < action_count; i++)
	{
		(void)sqlite3_prepare_v2(db, sql_write, sql_write_len, &stmt, NULL);
		(void)sqlite3_bind_int(stmt, 1, i);
		(void)sqlite3_bind_text(stmt, 2, action_names[i], -1, NULL);
		(void)sqlite3_bind_int(stmt, 3, keys[i]);
		(void)sqlite3_bind_text(stmt, 4, SDL_GetKeyName(keys[i]), -1, NULL);
		(void)sqlite3_bind_int(stmt, 5, controller_buttons[i]);
		(void)sqlite3_bind_text(stmt, 6, controller_button_names[i], -1, NULL);
		(void)sqlite3_step(stmt);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			SDL_Log("Failed to write action %s to database\n", action_names[i]);
	}

	(void)sqlite3_prepare_v2(db, sql_read, (int)SDL_strlen(sql_read), &stmt, NULL);
	for (i = 0; i < action_count; i++)
	{
		if (sqlite3_step(stmt) != SQLITE_ROW)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to read keymap for action: %s\n", action_names[i]);
			(void)sqlite3_finalize(stmt);
			continue;
		}
		keys[i] = sqlite3_column_int(stmt, 0);
		controller_buttons[i] = sqlite3_column_int(stmt, 1);
	}
	(void)sqlite3_finalize(stmt);
}

int
Amphora_UpdateKeymap(const char *action, SDL_Keycode keycode)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "UPDATE key_map SET key=?, key_name=? WHERE action=?;";
	const char *keyname = SDL_GetKeyName(keycode);

	(void)sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	(void)sqlite3_bind_int(stmt, 1, keycode);
	(void)sqlite3_bind_text(stmt, 2, keyname, -1, NULL);
	(void)sqlite3_bind_text(stmt, 3, action, -1, NULL);
	(void)sqlite3_step(stmt);
	if (sqlite3_finalize(stmt) != SQLITE_OK) return AMPHORA_STATUS_FAIL_UNDEFINED;

	return AMPHORA_STATUS_OK;
}

bool
Amphora_ObjectClicked(void *obj, int button, void (*callback)(void))
{
	int x, y;
	Uint32 flags;
	SDL_FRect *rect;
	IAmphoraObject *obj_generic = obj;
	Camera camera = Amphora_GetCamera();

	switch (obj_generic->type)
	{
		case AMPH_OBJ_SPR:
			rect = &((AmphoraImage *)obj)->rectangle;
			break;
		case AMPH_OBJ_TXT:
			rect = &((AmphoraString *)obj)->rectangle;
			break;
		default:
			return false;
	}

	flags = SDL_GetMouseState(&x, &y);
	if (flags != (Uint32)SDL_BUTTON(button)) return false;

	if (SDL_PointInFRect(&(SDL_FPoint){ (float)x + camera.x, (float)y + camera.y }, rect))
	{
		if (callback) callback();
		return true;
	}

	return false;
}

bool
Amphora_ObjectHover(void *obj)
{
	int x;
	int y;
	SDL_FRect *rect;
	IAmphoraObject *obj_generic = obj;
	Camera camera = Amphora_GetCamera();

	switch (obj_generic->type)
	{
		case AMPH_OBJ_SPR:
			rect = &((AmphoraImage *)obj)->rectangle;
			break;
		case AMPH_OBJ_TXT:
			rect = &((AmphoraString *)obj)->rectangle;
			break;
		default:
			return false;
	}

	(void)SDL_GetMouseState(&x, &y);

	return SDL_PointInFRect(&(SDL_FPoint){ (float)x + camera.x, (float)y + camera.y }, rect);
}

SDL_Keycode
Amphora_GetPressedKey(void)
{
	return pressed_key;
}

bool
Amphora_LeftJoystickActive(void)
{
	return joystickl_active;
}

bool
Amphora_RightJoystickActive(void)
{
	return joystickr_active;
}

Vector2f
Amphora_GetLeftJoystickState(void)
{
	return joystickl_state;
}

Vector2f
Amphora_GetRightJoystickState(void)
{
	return joystickr_state;
}

const char *
Amphora_GetActionKeyName(const char *action)
{
	sqlite3 *db = Amphora_GetDB();
	sqlite3_stmt *stmt;
	const char *sql = "SELECT key_name FROM key_map WHERE action=?;";
	char *key_name_r = NULL;

	Amphora_ValidatePtrNotNull(action, NULL);
	(void)sqlite3_prepare_v2(db, sql, (int)SDL_strlen(sql), &stmt, NULL);
	(void)sqlite3_bind_text(stmt, 1, action, -1, NULL);
	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		(void)sqlite3_finalize(stmt);
		return NULL;
	}
	key_name_r = Amphora_HeapStrdupFrame((const char *)sqlite3_column_text(stmt, 1));
	if (!key_name_r)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate space for string\n");
		(void)sqlite3_finalize(stmt);
		return NULL;
	}
	(void)sqlite3_finalize(stmt);

	return key_name_r;
}

void
Amphora_ForEachAction(void (*callback)(const char *, int))
{
	int i;

	if (!callback) return;

	for (i = 0; i < action_count; i++)
	{
		callback(action_names[i], i);
	}
}

/*
 * Internal functions
 */

int
Amphora_InitInput(void)
{
	sqlite3 *db = Amphora_GetDB();
	const char *sql = "CREATE TABLE IF NOT EXISTS key_map("
			  "idx INT NOT NULL PRIMARY KEY,"
			  "action TEXT NOT NULL,"
			  "key INT,"
			  "key_name TEXT,"
			  "gamepad INT,"
			  "gamepad_name TEXT);";
	char *err_msg = NULL;

	(void)sqlite3_exec(db, sql, NULL, NULL, &err_msg);
	if (err_msg)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", err_msg);
		sqlite3_free(err_msg);
		return -1;
	}

	return 0;
}

struct input_state_t *
Amphora_GetKeyActionState(void)
{
	return (struct input_state_t *)key_actions;
}

void
Amphora_AddController(Sint32 idx)
{
	if (controller) return;

	controller = SDL_GameControllerOpen(idx);
#ifdef DEBUG
	SDL_Log("Connected controller %d\n", SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)));
#endif
}

void
Amphora_RemoveController(SDL_JoystickID id)
{
	if (id != SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) return;

	SDL_GameControllerClose(controller);
	controller = NULL;
#ifdef DEBUG
	SDL_Log("Disconnected controller %d\n", id);
#endif
}

void
Amphora_ReleaseControllers(void)
{
	SDL_GameControllerClose(controller);
	controller = NULL;
}

bool
Amphora_ControllerConnected(void)
{
	return controller != NULL;
}

void
Amphora_HandleKeyDown(const SDL_Event *e)
{
	int i;

	pressed_key = e->key.keysym.sym;
	for (i = 0; i < action_count; i++)
	{
		if (e->key.keysym.sym == keys[i])
		{
			*key_actions |= (1LL << i);
			return;
		}
	}
}

void
Amphora_HandleKeyUp(const SDL_Event *e)
{
	int i;

	if (pressed_key == e->key.keysym.sym) pressed_key = 0;
	for (i = 0; i < action_count; i++)
	{
		if (e->key.keysym.sym == keys[i])
		{
			*key_actions &= ~(1LL << i);
			return;
		}
	}
}

void
Amphora_HandleGamepadDown(const SDL_Event *e)
{
	int i;

	for (i = 0; i < action_count; i++)
	{
		if (e->cbutton.button == controller_buttons[i])
		{
			*key_actions |= (1LL << i);
			return;
		}
	}
}

void
Amphora_HandleGamepadUp(const SDL_Event *e)
{
	int i;

	for (i = 0; i < action_count; i++)
	{
		if (e->cbutton.button == controller_buttons[i])
		{
			*key_actions &= ~(1LL << i);
			return;
		}
	}
}

void
Amphora_HandleJoystick(void)
{
	Amphora_ProcessJoystickState(SDL_CONTROLLER_AXIS_LEFTX, SDL_CONTROLLER_AXIS_LEFTY,
				     &joystickl_state, &joystickl_active);
	Amphora_ProcessJoystickState(SDL_CONTROLLER_AXIS_RIGHTX, SDL_CONTROLLER_AXIS_RIGHTY,
				     &joystickr_state, &joystickr_active);
}

/*
 * Private functions
 */

static void
Amphora_ProcessJoystickState(SDL_GameControllerAxis ax, SDL_GameControllerAxis ay, Vector2f *js, bool *jactive)
{
	float deadzone = 0.15f;
	float x;
	float y;
	float magnitude;
	float scale;
	Sint16 raw_x;
	Sint16 raw_y;

	raw_x = SDL_GameControllerGetAxis(controller, ax);
	raw_y = SDL_GameControllerGetAxis(controller, ay);
	x = raw_x < 0 ? (float)raw_x / 32768.0f : (float)raw_x / 32767.0f;
	y = raw_y < 0 ? (float)raw_y / 32768.0f : (float)raw_y / 32767.0f;
	magnitude = SDL_sqrtf(x * x + y * y);

	if (magnitude <= deadzone)
	{
		*jactive = false;
		(void)SDL_memset(js, 0, sizeof(Vector2f));
		return;
	}
	*jactive = true;
	scale = (magnitude - deadzone) / (1.0f - deadzone);

	js->x = x / magnitude * scale;
	js->y = y / magnitude * scale;
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterActionData(unsigned int *key_state, const char **actions, SDL_Keycode *keyboard_names, SDL_GameControllerButton *gamepad_buttons, const char **gamepad_button_names, int count)
{
	key_actions = key_state;
	action_names = actions;
	keys = keyboard_names;
	controller_buttons = gamepad_buttons;
	controller_button_names = gamepad_button_names;
	action_count = count;
}
