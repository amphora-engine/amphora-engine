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

/* File-scoped variables */
static HT_HashTable scenes;
static const AmphoraScene *scene_structs;
static const char **scene_names;
static int scenes_count;
static long current_scene_idx = 0;
static int current_scene_name = 0;
static AmphoraFader transition_fader;
static SDL_Color fade_color = { 0, 0, 0, 255 };
static SDL_Rect fade_rect;
static bool scene_update_lock = false;

int
Amphora_LoadScene(const char *name)
{
	Vector2 screen_size = Amphora_GetResolution();
	long idx;
	int i;

	idx = HT_GetValue(name, scenes);
	if (idx == -1)
	{
		Amphora_SetError(AMPHORA_STATUS_FAIL_UNDEFINED, "No scene %s", name);
		return AMPHORA_STATUS_FAIL_UNDEFINED;
	}
	current_scene_name = (int)idx;

	if (Amphora_RegisterEvent("amph_internal_scene_transition", Amphora_SceneTransitionEvent) == AMPHORA_STATUS_FAIL_UNDEFINED)
	{
		Amphora_SetError(AMPHORA_STATUS_FAIL_UNDEFINED, "Scene transition event registration failed");
		return AMPHORA_STATUS_FAIL_UNDEFINED;
	}
	Amphora_LockSceneUpdate();
	fade_rect.w = screen_size.x;
	fade_rect.h = screen_size.y;
	transition_fader.frames = transition_fader.timer * Amphora_GetFPS() / 1000;
	if (!((transition_fader.steps = Amphora_HeapAlloc((transition_fader.frames >> 1) * sizeof(Uint8), MEM_MISC))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate memory for fade steps\n");
		Amphora_UnlockSceneUpdate();
		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	for (i = 0; i < transition_fader.frames >> 1; i++)
	{
		transition_fader.steps[i] = i * 255 / ((transition_fader.frames >> 1) - 1);
	}
	transition_fader.idx = 0;
	transition_fader.idx_mod = 1;

	return AMPHORA_STATUS_OK;
}

int
Amphora_SetSceneFadeParameters(int ms, AmphoraColor color)
{
	transition_fader.timer = ms;
	fade_color.r = color.r;
	fade_color.g = color.g;
	fade_color.b = color.b;

	return AMPHORA_STATUS_OK;
}

/*
 * Internal functions
 */

void
Amphora_InitSceneManager(void)
{
	int i;

	scenes = HT_NewTable();
	for (i = 0; i < scenes_count; i++)
	{
		(void)HT_SetValue(scene_names[i], i, scenes);
	}
}

void
Amphora_DeInitSceneManager(void)
{
	HT_FreeTable(scenes);
}

void
Amphora_InitScene(void)
{
	scene_structs[current_scene_idx].init_func();
	Amphora_UnlockSceneUpdate();
}

void
Amphora_UpdateScene(unsigned int frame_count)
{
	scene_structs[current_scene_idx].update_func(frame_count, Amphora_GetKeyActionState());
}

void
Amphora_DestroyScene(void)
{
	scene_structs[current_scene_idx].destroy_func();
	Amphora_DestroyCurrentMap();
	Amphora_FreeObjectGroup();
	Amphora_FreeAllSFX();
	Amphora_ResetRenderList();
	Amphora_UnboundCamera();
	Amphora_FreeAllIMG();
	Amphora_FreeAllFonts();
}

bool
Amphora_IsSceneUpdateLocked(void)
{
	return scene_update_lock;
}

void
Amphora_LockSceneUpdate(void)
{
#ifdef DEBUG
	SDL_Log("Scene update locked\n");
#endif
	scene_update_lock = true;
}

void
Amphora_UnlockSceneUpdate(void)
{
#ifdef DEBUG
	SDL_Log("Scene update unlocked\n");
#endif
	scene_update_lock = false;
}

/*
 * Private functions
 */

static void
Amphora_SceneTransitionEvent(void)
{
	SDL_Renderer *renderer = Amphora_GetRenderer();

	if (!transition_fader.timer)
	{
		Amphora_DestroyScene();
		current_scene_idx = HT_GetValue(scene_names[current_scene_name], scenes);
		Amphora_InitScene();
		(void)Amphora_UnregisterEvent("amph_internal_scene_transition");
		return;
	}
	(void)SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	(void)SDL_SetRenderDrawColor(renderer, fade_color.r, fade_color.g, fade_color.b, transition_fader.steps[transition_fader.idx]);
	(void)SDL_RenderFillRect(renderer, &fade_rect);
	transition_fader.idx += transition_fader.idx_mod;
	if (transition_fader.idx == (transition_fader.frames >> 1) - 1)
	{
		transition_fader.idx_mod = -1;
		Amphora_DestroyScene();
		current_scene_idx = HT_GetValue(scene_names[current_scene_name], scenes);
		Amphora_InitScene();
		Amphora_SetCamera(0, 0);
	}
	if (transition_fader.idx == 0 && transition_fader.idx_mod == -1)
	{
		Amphora_HeapFree(transition_fader.steps);
		(void)Amphora_UnregisterEvent("amph_internal_scene_transition");
	}
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterSceneData(const AmphoraScene *scenes_list, const char **names, int count)
{
	scene_structs = scenes_list;
	scene_names = names;
	scenes_count = count;
}
