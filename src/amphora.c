#include "util.h"
#include "internal/context.h"
#include "internal/db.h"
#include "internal/error.h"
#include "internal/events.h"
#include "internal/img.h"
#include "internal/input.h"
#include "internal/memory.h"
#include "internal/mixer.h"
#include "internal/particles.h"
#include "internal/prefs.h"
#include "internal/random.h"
#include "internal/render.h"
#include "internal/save_data.h"
#include "internal/scenes.h"
#include "internal/session_data.h"
#include "internal/system.h"
#include "internal/tilemap.h"
#include "internal/ttf.h"

/* Prototypes for private functions */
static int Amphora_MainLoop(SDL_Event *e);
static void Amphora_SaveConfig(void);
static void Amphora_CleanResources(void);

static struct amphora_ctx init;
static struct amphora_ctx *inst = &init;

int
Amphora_StartEngine(void)
{
	SDL_Event e;

	/*
	 * TODO: check Init* error codes
	 */
	Amphora_InitHeap();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL, "Failed to init SDL: %s", SDL_GetError());
		return AMPHORA_STATUS_CORE_FAIL;
	}

	if (IMG_Init(IMG_INIT_PNG) < 0)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL, "Failed to init SDL_image: %s", SDL_GetError());
		return AMPHORA_STATUS_CORE_FAIL;
	}

	if (Mix_Init(MIX_INIT_OGG) < 0)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL, "Failed to init SDL_mixer: %s", SDL_GetError());
		return AMPHORA_STATUS_CORE_FAIL;
	}
	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL, "Failed to open audio device: %s", SDL_GetError());
		return AMPHORA_STATUS_CORE_FAIL;
	}
	if (Amphora_InitSFX() == -1)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL, "Failed to load sfx data");
		return AMPHORA_STATUS_CORE_FAIL;
	}
	if (Amphora_InitMusic() == -1)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL, "Failed to load music data");
		return AMPHORA_STATUS_CORE_FAIL;
	}
	if (TTF_Init() < 0)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL, "Failed to init SDL_ttf: %s", SDL_GetError());
		return AMPHORA_STATUS_CORE_FAIL;
	}
	if (Amphora_InitFonts() == -1)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL,"Failed to load TTF font data");
		return AMPHORA_STATUS_CORE_FAIL;
	}
	if (Amphora_InitMaps() == -1)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL,"Failed to load tilemap data");
		return AMPHORA_STATUS_CORE_FAIL;
	}
	Amphora_InitRand();
	Amphora_InitDB();
	Amphora_InitConfig();
	Amphora_InitEvents();
	if (Amphora_InitRender() == -1)
	{
		Amphora_SetError(AMPHORA_STATUS_CORE_FAIL,"Failed to init renderer");
		return AMPHORA_STATUS_CORE_FAIL;
	}
	Amphora_InitSave();
	Amphora_InitSessionData();
	Amphora_InitInput();
	Amphora_LoadKeymapV1();
	Amphora_InitSceneManager();
	Amphora_InitParticleSystem();

	inst->engine_flags.engine_running = true;
	inst->framerate = (uint32_t)Amphora_LoadFPS();

	Amphora_InitScene();

	while (Amphora_MainLoop(&e) == 0) {}
	Amphora_SaveConfig();
	Amphora_CleanResources();
	IMG_Quit();
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
	inst->engine_flags.engine_running = false;

	return AMPHORA_STATUS_OK;
}

bool
Amphora_IsEngineRunning(void)
{
	return inst->engine_flags.engine_running;
}

void
Amphora_QuitGameV1(void)
{
	inst->engine_flags.quit_requested = true;
}

unsigned int
Amphora_GetFrameV1(void)
{
	return inst->frame_count;
}

int
Amphora_GetFPSV1(void)
{
	return (int)inst->framerate;
}

const unsigned int *
Amphora_GetFrameAddress(void)
{
	return &inst->frame_count;
}

unsigned int
Amphora_GetTicksV1(void)
{
	return SDL_GetTicks();
}

/*
 * Private functions
 */

static int
Amphora_MainLoop(SDL_Event *e)
{
	Uint32 frame_start;
	Uint32 frame_end;
	Uint32 frame_time;
	Uint32 remaining_time;

	frame_start = SDL_GetTicks();
	inst->frame_count++;

	if (Amphora_ProcessEventLoop(e) == SDL_QUIT) inst->engine_flags.quit_requested = true;
	if (inst->engine_flags.quit_requested)
		return 1;

	Amphora_ClearBG();
	if (Amphora_IsSceneUpdateLocked() == false)
	{
		if (Amphora_ControllerConnected()) Amphora_HandleJoystick();
		Amphora_UpdateScene();
		Amphora_UpdateCamera();
	}
	Amphora_ProcessRenderList();
	Amphora_ProcessRegisteredEvents();
	Amphora_HeapClearFrameHeap();

	SDL_RenderPresent(Amphora_GetRenderer());

	frame_end = SDL_GetTicks();
	frame_time = frame_end - frame_start;
	if (frame_time < 1000 / inst->framerate)
	{
		remaining_time = 1000 / inst->framerate - frame_time;
		remaining_time = Amphora_HeapHousekeeping(remaining_time);
		SDL_Delay(remaining_time);
#ifdef DEBUG
	}
	else if (frame_time > 1000 / inst->framerate)
	{
		SDL_Log("Lag on frame %u (frame took %u ticks, %d ticks per frame)\n", inst->frame_count, frame_time, 1000 / inst->framerate);
	}
#else
	}
#endif

	return 0;
}

static void
Amphora_SaveConfig(void)
{
	/*
	 * TODO: Check Save* error codes
	 */
	Vector2 win_size = Amphora_GetResolutionV1();
	Uint32 win_flags = SDL_GetWindowFlags(Amphora_GetWindow());

	if (!Amphora_IsWindowFullscreenV1())
	{
		Amphora_SaveWinX(win_size.x);
		Amphora_SaveWinY(win_size.y);
	}
	Amphora_SaveWinFlags(win_flags);
	Amphora_SaveFPS(inst->framerate);
}

static void
Amphora_CleanResources(void)
{
	Amphora_DestroyScene();
	Amphora_DeInitSceneManager();
	Amphora_DeInitEvents();
	Amphora_CloseIMG();
	Amphora_CloseFonts();
	Amphora_CloseSFX();
	Amphora_CloseMusic();
	Amphora_FreeAllObjectGroups();
	Amphora_CloseMapHashTables();
	Amphora_DeInitSessionData();
	Amphora_CloseRender();
	Amphora_ReleaseControllers();
	Amphora_CloseDB();
	Amphora_DestroyHeap();
}
