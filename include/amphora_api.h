#ifndef AMPHORA_AMPHORA_API_H
#define AMPHORA_AMPHORA_API_H

#include "internal/scenes.h"

#include "SDL.h"

#define AMPHORA_API_VERSION 1

extern int Amphora_StartEngine(void);
extern void Amphora_RegisterGameData(const char *, const char *);
extern void Amphora_RegisterWindowTitle(const char *);
extern void Amphora_RegisterPrefs(const char *, const char *, int, int, unsigned int, int);
extern void Amphora_RegisterActionData(unsigned int *, const char **, SDL_Keycode *, SDL_GameControllerButton *, const char **, int);
extern void Amphora_RegisterSceneData(const AmphoraScene *, const char **, int);
extern void Amphora_RegisterImageData(const char **, const char **, int);
extern void Amphora_RegisterFontData(const char **, const char **, int);
extern void Amphora_RegisterMapData(const char **, const char **, int);
extern void Amphora_RegisterSFXData(const char **, const char **, int);
extern void Amphora_RegisterMusicData(const char **, const char **, int);

typedef struct
{
	char *engine_version;
	int api_version;
	int (*StartEngine)(void);
	void (*RegisterGameData)(const char *, const char *);
	void (*RegisterWindowTitle)(const char *);
	void (*RegisterPrefs)(const char *, const char *, int, int, unsigned int, int);
	void (*RegisterActionData)(Uint32 *, const char **, SDL_Keycode *, SDL_GameControllerButton *, const char **, int);
	void (*RegisterSceneData)(const AmphoraScene *, const char **, int);
	void (*RegisterImageData)(const char **, const char **, int);
	void (*RegisterFontData)(const char **, const char **, int);
	void (*RegisterMapData)(const char **, const char **, int);
	void (*RegisterSFXData)(const char **, const char **, int);
	void (*RegisterMusicData)(const char **, const char **, int);
	void *(*GetAPI)(int);
} AmphoraStartup;

typedef struct
{
#define AMPHORA_VFUNCTION_V1(ret, name, sig_args, p_sig_args, call_args) ret (*name) p_sig_args
#define AMPHORA_FUNCTION_V1(ret, name, sig_args, call_args) ret (*name) sig_args
#define AMPHORA_ROUTINE_V1(name, sig_args, call_args) void (*name) sig_args
	#include "amphora_api.def"
#undef AMPHORA_VFUNCTION_V1
#undef AMPHORA_FUNCTION_V1
#undef AMPHORA_ROUTINE_V1
} AmphoraAPI_V1;

#endif /* AMPHORA_AMPHORA_API_H */
