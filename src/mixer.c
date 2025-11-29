#include "internal/mixer.h"
#include "internal/ht_hash.h"

/* Prototypes for private functions */
static void Amphora_FreeMusic(void);

/* File-scoped variables */
static HT_HashTable sfx;
static HT_HashTable music;
static HT_HashTable open_sfx;
static Mix_Music *current_music;
static const char **sfx_names;
static const char **sfx_paths;
static int sfx_count;
static const char **music_names;
static const char **music_paths;
static int music_count;

void
Amphora_PlaySFX(const char *name, const int channel, const int repeat)
{
	SDL_RWops *sfx_rw = NULL;
	Mix_Chunk *sfx_chunk = NULL;
	int v;

	if (HT_GetValue(name, open_sfx) == -1)
	{
#ifdef DEBUG
		SDL_Log("Loading sfx: %s\n", name);
#endif
		sfx_rw = SDL_RWFromFile(HT_GetRef(name, char, sfx), "rb");
		HT_StoreRef(name, Mix_LoadWAV_RW(sfx_rw, 1), open_sfx);
	}
	if (channel > -1 && Mix_Playing(channel)) return;
	sfx_chunk = HT_GetRef(name, Mix_Chunk, open_sfx);

	if (Mix_VolumeChunk(sfx_chunk, -1) != (v = (int)HT_GetStatus(name, sfx)))
		Mix_VolumeChunk(sfx_chunk, v);

	(void)Mix_PlayChannel(channel, sfx_chunk, repeat);
}

void
Amphora_SetSFXVolume(const char *name, int volume)
{
	if (HT_GetValue(name, sfx) == -1) return;

	(void)HT_SetStatus(name, volume, sfx);
}

void
Amphora_SetMusic(const char *name)
{
	SDL_RWops *mus_rw = SDL_RWFromFile(HT_GetRef(name, char, music), "rb");

	if (Mix_PlayingMusic())
	{
		(void)Mix_HaltMusic();
		Mix_FreeMusic(current_music);
		current_music = NULL;
	}

	(void)SDL_RWseek(mus_rw, 0, RW_SEEK_SET);
	current_music = Mix_LoadMUS_RW(mus_rw, 1);
}

void
Amphora_PlayMusic(int ms)
{
	if (Mix_PlayingMusic()) return;

	(void)Mix_FadeInMusic(current_music, -1, ms);
}

void
Amphora_PlayMusicN(int n, int ms)
{
	if (Mix_PlayingMusic()) return;

	(void)Mix_FadeInMusic(current_music, n, ms);
}

void
Amphora_PauseMusic(void)
{
	if (!Mix_PlayingMusic()) return;

	Mix_PauseMusic();
}

void
Amphora_UnpauseMusic(void)
{
	if (!Mix_PausedMusic()) return;

	Mix_ResumeMusic();
}

void
Amphora_StopMusic(void)
{
	if (!Mix_PlayingMusic()) return;

	(void)Mix_HaltMusic();
}

void
Amphora_FadeOutMusic(int ms)
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

	sfx = HT_NewTable();
	open_sfx = HT_NewTable();
	for (i = 0; i < sfx_count; i++)
	{
		HT_StoreRef(sfx_names[i], sfx_paths[i], sfx);
		(void)HT_SetStatus(sfx_names[i], MIX_MAX_VOLUME, sfx);
#ifdef DEBUG
		SDL_Log("Found sfx %s\n", sfx_names[i]);
#endif
	}

	return 0;
}

int
Amphora_InitMusic(void)
{
	int i;

	music = HT_NewTable();
	for (i = 0; i < music_count; i++)
	{
		HT_StoreRef(music_names[i], music_paths[i], music);
#ifdef DEBUG
		SDL_Log("Found music %s\n", music_names[i]);
#endif
	}
	Mix_HookMusicFinished(Amphora_FreeMusic);

	return 0;
}

void
Amphora_FreeAllSFX(void)
{
	int i;

	for (i = 0; i < sfx_count; i++)
	{
		if (HT_GetValue(sfx_names[i], open_sfx) != -1)
		{
#ifdef DEBUG
			SDL_Log("Unloading sfx: %s\n", sfx_names[i]);
#endif
			Mix_FreeChunk(HT_GetRef(sfx_names[i], Mix_Chunk, open_sfx));
			(void)HT_SetValue(sfx_names[i], 0, open_sfx);
			HT_DeleteKey(sfx_names[i], open_sfx);
		}
	}
}

void
Amphora_CloseSFX(void)
{
	Amphora_FreeAllSFX();
	HT_FreeTable(open_sfx);
	HT_FreeTable(sfx);
}

void
Amphora_CloseMusic(void)
{
	if (current_music && !Mix_FadingMusic())
	{
		Amphora_FreeMusic();
	}
	HT_FreeTable(music);
}

/*
 * Private functions
 */

static void
Amphora_FreeMusic(void)
{
	Mix_FreeMusic(current_music);
	current_music = NULL;
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterSFXData(const char **names, const char **paths, int count)
{
	sfx_names = names;
	sfx_paths = paths;
	sfx_count = count;
}

void
Amphora_RegisterMusicData(const char **names, const char **paths, int count)
{
	music_names = names;
	music_paths = paths;
	music_count = count;
}
