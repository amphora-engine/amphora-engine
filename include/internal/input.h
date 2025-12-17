#ifndef INPUT_INTERNAL_H
#define INPUT_INTERNAL_H

#include "../input.h"

int Amphora_InitInput(void);
void Amphora_AddController(Sint32 idx);
void Amphora_RemoveController(SDL_JoystickID id);
void Amphora_ReleaseControllers(void);
bool Amphora_ControllerConnected(void);
void Amphora_HandleKeyDown(const SDL_Event *e);
void Amphora_HandleKeyUp(const SDL_Event *e);
void Amphora_HandleGamepadDown(const SDL_Event *e);
void Amphora_HandleGamepadUp(const SDL_Event *e);
void Amphora_HandleJoystick(void);

#endif /* INPUT_INTERNAL_H */
