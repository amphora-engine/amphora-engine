#ifndef UNTITLED_PLATFORMER_UTIL_H
#define UNTITLED_PLATFORMER_UTIL_H

#include "SDL.h"

#define Ampohra_Max(a,b) ((a) > (b) ? (a) : (b))
#define Ampohra_Min(a,b) ((a) < (b) ? (a) : (b))
#define Amphora_IsOdd(a) ((a) & 1)
#define Amphora_IsEven(a) (!Amphora_IsOdd((a)))

#define fullscreen SDL_WINDOW_FULLSCREEN_DESKTOP
#define fixed SDL_WINDOW_SHOWN
#define resizable SDL_WINDOW_RESIZABLE

#ifndef __cplusplus
#ifndef bool
#define bool SDL_bool
#endif
#ifndef true
#define true SDL_TRUE
#endif
#ifndef false
#define false SDL_FALSE
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
void Amphora_QuitGame(void); /* Request to quit the game */
Uint32 Amphora_GetFrame(void); /* Get the current running framerate */
Uint32 Amphora_GetFPS(void); /* Get the current running framerate */
Uint32 Amphora_GetTicks(void); /* Get the current running time in milliseconds */
#ifdef __cplusplus
}
#endif

#endif /* UNTITLED_PLATFORMER_UTIL_H */
