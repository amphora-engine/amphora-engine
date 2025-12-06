#ifndef AMPHORA_SCENES_H
#define AMPHORA_SCENES_H

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif
int Amphora_LoadScene(const char *name);
int Amphora_SetSceneFadeParameters(int ms, AmphoraColor color);
#ifdef __cplusplus
}
#endif

#endif /* AMPHORA_SCENES_H */
