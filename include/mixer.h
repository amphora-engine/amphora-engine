#ifndef AMPHORA_MIXER_H
#define AMPHORA_MIXER_H

#include "SDL.h"
#include "SDL_mixer.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Play a sound effect */
void Amphora_PlaySFXV1(const char *name, int channel, int repeat);
/* Sets the volume for a sound effect */
void Amphora_SetSFXVolumeV1(const char *name, int volume);
/* Sets the current music track */
void Amphora_SetMusicV1(const char *name);
/* Play the current set music track with a fade-in, looping infinitely */
void Amphora_PlayMusicV1(int ms);
/* Play the current set music track with a fade-in, looping n times */
void Amphora_PlayMusicNV1(int n, int ms);
/* Pause the currently playing music track */
void Amphora_PauseMusicV1(void);
/* Unpause the currently playing music track if paused */
void Amphora_UnpauseMusicV1(void);
/* Stop the currently playing music track and free its resources immediately */
void Amphora_StopMusicV1(void);
/* Stop the currently playing music track and free its resources after a fade-out */
void Amphora_FadeOutMusicV1(int ms);
#ifdef __cplusplus
};
#endif

#endif /* AMPHORA_MIXER_H */
