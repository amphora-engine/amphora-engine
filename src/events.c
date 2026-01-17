#include "internal/context.h"
#include "internal/error.h"
#include "internal/events.h"
#include "internal/ht_hash.h"
#include "internal/input.h"
#include "internal/memory.h"
#include "internal/render.h"
#include "internal/scenes.h"

static struct events_ctx init = { .ev_max = EVENT_BLOCK_SIZE };
static struct events_ctx *inst = &init;

int
Amphora_RegisterEventV1(const char *name, void (*func)(void))
{
	int i;

	if (Amphora_IsSceneUpdateLocked()) return AMPHORA_STATUS_OK;

	if (HT_GetValue(name, inst->ev_table) != -1)
	{
		Amphora_SetError(AMPHORA_STATUS_FAIL_UNDEFINED, "Event %s is already used", name);
		return AMPHORA_STATUS_FAIL_UNDEFINED;
	}

#ifdef DEBUG
	SDL_Log("Registering event: %s\n", name);
#endif
	if (++inst->ev_count >= inst->ev_max)
	{
		inst->ev_names = Amphora_HeapRealloc(inst->ev_names, inst->ev_max * sizeof(char *) + EVENT_BLOCK_SIZE * sizeof(char *), MEM_STRING);
		(void)SDL_memset(inst->ev_names + inst->ev_max, 0, EVENT_BLOCK_SIZE * sizeof(char *));
		inst->ev_max += EVENT_BLOCK_SIZE;
	}
	for (i = 0; i < inst->ev_max; i++)
	{
		if (!inst->ev_names[i])
		{
			inst->ev_names[i] = Amphora_HeapStrdup(name);
			HT_StoreRef(name, func, inst->ev_table);
			HT_SetStatus(name, i, inst->ev_table);
			break;
		}
	}

	return AMPHORA_STATUS_OK;
}

int
Amphora_UnregisterEventV1(const char *name)
{
	int i = (int)HT_GetStatus(name, inst->ev_table);

	if (i == -1)
	{
		Amphora_SetError(AMPHORA_STATUS_FAIL_UNDEFINED, "Event %s is not registered", name);
		return AMPHORA_STATUS_FAIL_UNDEFINED;
	}

#ifdef DEBUG
	SDL_Log("Unregistering event: %s\n", name);
#endif
	inst->ev_count--;
	Amphora_HeapFree(inst->ev_names[i]);
	inst->ev_names[i] = NULL;
	HT_StoreRef(name, NULL, inst->ev_table);
	HT_DeleteKey(name, inst->ev_table);

	return AMPHORA_STATUS_OK;
}

/*
 * Internal functions
 */

void
Amphora_InitEvents(void)
{
	if (!((inst->ev_names = Amphora_HeapCalloc(EVENT_BLOCK_SIZE, sizeof(char *), MEM_STRING))))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate event name table");
		return;
	}
	inst->ev_table = HT_NewTable();
}

void
Amphora_DeInitEvents(void)
{
	int i;

	for (i = 0; i < inst->ev_count; i++)
	{
		if (inst->ev_names[i]) Amphora_HeapFree(inst->ev_names[i]);
	}
	Amphora_HeapFree(inst->ev_names);
	HT_FreeTable(inst->ev_table);
}

Uint32
Amphora_ProcessEventLoop(SDL_Event *e)
{
	while (SDL_PollEvent(e))
	{
		switch (e->type)
		{
			case SDL_QUIT:
				return e->type;
			case SDL_KEYDOWN:
				Amphora_HandleKeyDown(e);
				break;
			case SDL_KEYUP:
				Amphora_HandleKeyUp(e);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				Amphora_HandleGamepadDown(e);
				break;
			case SDL_CONTROLLERBUTTONUP:
				Amphora_HandleGamepadUp(e);
				break;
			case SDL_CONTROLLERDEVICEADDED:
				Amphora_AddController(e->cdevice.which);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				Amphora_RemoveController(e->cdevice.which);
				break;
			case SDL_WINDOWEVENT:
				if (e->window.event != SDL_WINDOWEVENT_RESIZED) break;

				Amphora_SetRenderLogicalSize(Amphora_GetResolutionV1());
				break;
			default:
				break;
		}
	}

	return AMPHORA_STATUS_OK;
}

void
Amphora_ProcessRegisteredEvents(void)
{
	int i;

	for (i = 0; i < inst->ev_max; i++)
	{
		if (!inst->ev_names[i]) continue;

		((void(*)(void))HT_GetValue(inst->ev_names[i], inst->ev_table))();
	}
}
