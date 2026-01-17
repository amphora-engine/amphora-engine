#include "internal/context.h"
#include "internal/mixer.h"
#include "internal/ht_hash.h"

/* Prototypes for private functions */
static void Amphora_FreeMusic(void);

static struct mixer_ctx init;
static struct mixer_ctx *inst = &init;

void
Amphora_PlaySFXV1(const char *name, const int channel, const int repeat)
{
	SDL_RWops *sfx_rw = NULL;
	Mix_Chunk *sfx_chunk = NULL;
	int v;

	if (HT_GetValue(name, inst->open_sfx) == -1)
	{
#ifdef DEBUG
		SDL_Log("Loading sfx: %s\n", name);
#endif
		sfx_rw = SDL_RWFromFile(HT_GetRef(name, char, inst->sfx), "rb");
		HT_StoreRef(name, Mix_LoadWAV_RW(sfx_rw, 1), inst->open_sfx);
	}
	if (channel > -1 && Mix_Playing(channel)) return;
	sfx_chunk = HT_GetRef(name, Mix_Chunk, inst->open_sfx);

	if (Mix_VolumeChunk(sfx_chunk, -1) != (v = (int)HT_GetStatus(name, inst->sfx)))
		Mix_VolumeChunk(sfx_chunk, v);

	(void)Mix_PlayChannel(channel, sfx_chunk, repeat);
}

void
Amphora_SetSFXVolumeV1(const char *name, int volume)
{
	if (HT_GetValue(name, inst->sfx) == -1) return;

	(void)HT_SetStatus(name, volume, inst->sfx);
}

void
Amphora_SetMusicV1(const char *name)
{
	SDL_RWops *mus_rw = SDL_RWFromFile(HT_GetRef(name, char, inst->music), "rb");

	if (Mix_PlayingMusic())
	{
		(void)Mix_HaltMusic();
		Mix_FreeMusic(inst->current_music);
		inst->current_music = NULL;
	}

	(void)SDL_RWseek(mus_rw, 0, RW_SEEK_SET);
	inst->current_music = Mix_LoadMUS_RW(mus_rw, 1);
}

void
Amphora_PlayMusicV1(int ms)
{
	if (Mix_PlayingMusic()) return;

	(void)Mix_FadeInMusic(inst->current_music, -1, ms);
}

void
Amphora_PlayMusicNV1(int n, int ms)
{
	if (Mix_PlayingMusic()) return;

	(void)Mix_FadeInMusic(inst->current_music, n, ms);
}

void
Amphora_PauseMusicV1(void)
{
	if (!Mix_PlayingMusic()) return;

	Mix_PauseMusic();
}

void
Amphora_UnpauseMusicV1(void)
{
	if (!Mix_PausedMusic()) return;

	Mix_ResumeMusic();
}

void
Amphora_StopMusicV1(void)
{
	if (!Mix_PlayingMusic()) return;

	(void)Mix_HaltMusic();
}

void
Amphora_FadeOutMusicV1(int ms)
{
	if (!Mix_PlayingMusic()) return;

	(void)Mix_FadeOutMusic(ms);
}

/*
 * Internal functions
 */

int
Amphora_InitSFX(void)
{
	int i;

	init.sfx = HT_NewTable();
	init.open_sfx = HT_NewTable();
	for (i = 0; i < init.sfx_count; i++)
	{
		HT_StoreRef(init.sfx_names[i], init.sfx_paths[i], init.sfx);
		(void)HT_SetStatus(init.sfx_names[i], MIX_MAX_VOLUME, init.sfx);
#ifdef DEBUG
		SDL_Log("Found sfx %s\n", init.sfx_names[i]);
#endif
	}

	return 0;
}

int
Amphora_InitMusic(void)
{
	int i;

	init.music = HT_NewTable();
	for (i = 0; i < init.music_count; i++)
	{
		HT_StoreRef(init.music_names[i], init.music_paths[i], init.music);
#ifdef DEBUG
		SDL_Log("Found music %s\n", init.music_names[i]);
#endif
	}
	Mix_HookMusicFinished(Amphora_FreeMusic);

	return 0;
}

void
Amphora_FreeAllSFX(void)
{
	int i;

	for (i = 0; i < inst->sfx_count; i++)
	{
		if (HT_GetValue(inst->sfx_names[i], inst->open_sfx) != -1)
		{
#ifdef DEBUG
			SDL_Log("Unloading sfx: %s\n", inst->sfx_names[i]);
#endif
			Mix_FreeChunk(HT_GetRef(inst->sfx_names[i], Mix_Chunk, inst->open_sfx));
			(void)HT_SetValue(inst->sfx_names[i], 0, inst->open_sfx);
			HT_DeleteKey(inst->sfx_names[i], inst->open_sfx);
		}
	}
}

void
Amphora_CloseSFX(void)
{
	Amphora_FreeAllSFX();
	HT_FreeTable(init.open_sfx);
	HT_FreeTable(init.sfx);
}

void
Amphora_CloseMusic(void)
{
	if (inst->current_music && !Mix_FadingMusic())
	{
		Amphora_FreeMusic();
	}
	HT_FreeTable(init.music);
}

/*
 * Private functions
 */

static void
Amphora_FreeMusic(void)
{
	Mix_FreeMusic(inst->current_music);
	inst->current_music = NULL;
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterSFXData(const char **names, const char **paths, int count)
{
	init.sfx_names = names;
	init.sfx_paths = paths;
	init.sfx_count = count;
}

void
Amphora_RegisterMusicData(const char **names, const char **paths, int count)
{
	init.music_names = names;
	init.music_paths = paths;
	init.music_count = count;
}
