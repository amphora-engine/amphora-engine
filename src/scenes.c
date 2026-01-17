#include "internal/context.h"
#include "internal/error.h"
#include "internal/events.h"
#include "internal/ht_hash.h"
#include "internal/img.h"
#include "internal/input.h"
#include "internal/lib.h"
#include "internal/memory.h"
#include "internal/mixer.h"
#include "internal/render.h"
#include "internal/scenes.h"
#include "internal/ttf.h"
#include "internal/tilemap.h"

/* Prototypes for private functions */
static void Amphora_SceneTransitionEvent(void);

static struct scenes_ctx init = { .fade_color = { 0, 0, 0, 0xff } };
static struct scenes_ctx *inst = &init;

int
Amphora_LoadSceneV1(const char *name)
{
	Vector2 screen_size = Amphora_GetResolutionV1();
	long idx;
	int i;

	idx = HT_GetValue(name, inst->scenes);
	if (idx == -1)
	{
		Amphora_SetError(AMPHORA_STATUS_FAIL_UNDEFINED, "No scene %s", name);
		return AMPHORA_STATUS_FAIL_UNDEFINED;
	}
	inst->current_scene_name = (int)idx;

	if (Amphora_RegisterEventV1("amph_internal_scene_transition", Amphora_SceneTransitionEvent) == AMPHORA_STATUS_FAIL_UNDEFINED)
	{
		Amphora_SetError(AMPHORA_STATUS_FAIL_UNDEFINED, "Scene transition event registration failed");
		return AMPHORA_STATUS_FAIL_UNDEFINED;
	}
	Amphora_LockSceneUpdate();
	inst->fade_rect.w = screen_size.x;
	inst->fade_rect.h = screen_size.y;
	inst->transition_fader.frames = inst->transition_fader.timer * Amphora_GetFPSV1() / 1000;
	if (!((inst->transition_fader.steps = Amphora_HeapAlloc((inst->transition_fader.frames >> 1) * sizeof(Uint8), MEM_MISC))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate memory for fade steps\n");
		Amphora_UnlockSceneUpdate();
		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	for (i = 0; i < inst->transition_fader.frames >> 1; i++)
	{
		inst->transition_fader.steps[i] = i * 255 / ((inst->transition_fader.frames >> 1) - 1);
	}
	inst->transition_fader.idx = 0;
	inst->transition_fader.idx_mod = 1;

	return AMPHORA_STATUS_OK;
}

int
Amphora_SetSceneFadeParametersV1(int ms, AmphoraColor color)
{
	inst->transition_fader.timer = ms;
	inst->fade_color.r = color.r;
	inst->fade_color.g = color.g;
	inst->fade_color.b = color.b;

	return AMPHORA_STATUS_OK;
}

/*
 * Internal functions
 */

void
Amphora_InitSceneManager(void)
{
	int i;

	init.scenes = HT_NewTable();
	for (i = 0; i < init.scenes_count; i++)
	{
		(void)HT_SetValue(init.scene_names[i], i, init.scenes);
	}
}

void
Amphora_DeInitSceneManager(void)
{
	HT_FreeTable(init.scenes);
}

void
Amphora_InitScene(void)
{
	inst->scene_structs[inst->current_scene_idx].init_func();
	Amphora_UnlockSceneUpdate();
}

void
Amphora_UpdateScene(void)
{
	inst->scene_structs[inst->current_scene_idx].update_func();
}

void
Amphora_DestroyScene(void)
{
	inst->scene_structs[inst->current_scene_idx].destroy_func();
	Amphora_DestroyCurrentMap();
	Amphora_FreeObjectGroup();
	Amphora_FreeAllSFX();
	Amphora_ResetRenderList();
	Amphora_UnboundCameraV1();
	Amphora_FreeAllIMG();
	Amphora_FreeAllFonts();
}

bool
Amphora_IsSceneUpdateLocked(void)
{
	return inst->scene_update_lock;
}

void
Amphora_LockSceneUpdate(void)
{
#ifdef DEBUG
	SDL_Log("Scene update locked\n");
#endif
	inst->scene_update_lock = true;
}

void
Amphora_UnlockSceneUpdate(void)
{
#ifdef DEBUG
	SDL_Log("Scene update unlocked\n");
#endif
	inst->scene_update_lock = false;
}

/*
 * Private functions
 */

static void
Amphora_SceneTransitionEvent(void)
{
	SDL_Renderer *renderer = Amphora_GetRenderer();

	if (!inst->transition_fader.timer)
	{
		Amphora_DestroyScene();
		inst->current_scene_idx = HT_GetValue(inst->scene_names[inst->current_scene_name], inst->scenes);
		Amphora_InitScene();
		(void)Amphora_UnregisterEventV1("amph_internal_scene_transition");
		return;
	}
	(void)SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	(void)SDL_SetRenderDrawColor(renderer, inst->fade_color.r, inst->fade_color.g, inst->fade_color.b, inst->transition_fader.steps[inst->transition_fader.idx]);
	(void)SDL_RenderFillRect(renderer, &inst->fade_rect);
	inst->transition_fader.idx += inst->transition_fader.idx_mod;
	if (inst->transition_fader.idx == (inst->transition_fader.frames >> 1) - 1)
	{
		inst->transition_fader.idx_mod = -1;
		Amphora_DestroyScene();
		inst->current_scene_idx = HT_GetValue(inst->scene_names[inst->current_scene_name], inst->scenes);
		Amphora_InitScene();
		Amphora_SetCameraV1(0, 0);
	}
	if (inst->transition_fader.idx == 0 && inst->transition_fader.idx_mod == -1)
	{
		Amphora_HeapFree(inst->transition_fader.steps);
		(void)Amphora_UnregisterEventV1("amph_internal_scene_transition");
	}
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterSceneData(const AmphoraScene *scenes_list, const char **names, int count)
{
	init.scene_structs = scenes_list;
	init.scene_names = names;
	init.scenes_count = count;
}
