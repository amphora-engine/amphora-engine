#ifndef UNTITLED_PLATFORMER_EVENTS_H
#define UNTITLED_PLATFORMER_EVENTS_H

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif
int Amphora_RegisterEventV1(const char *name, void (*func)(void));
int Amphora_UnregisterEventV1(const char *name);
#ifdef __cplusplus
}
#endif

#endif /* UNTITLED_PLATFORMER_EVENTS_H */
