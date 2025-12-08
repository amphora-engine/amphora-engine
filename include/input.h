#ifndef UNTITLED_PLATFORMER_INPUT_H
#define UNTITLED_PLATFORMER_INPUT_H

#include "SDL.h"

#include "img.h"

#ifdef __cplusplus
extern "C" {
#endif
void Amphora_LoadKeymapV1(void);
/* Change the keymap for an action */
int Amphora_UpdateKeymapV1(const char *action, SDL_Keycode keycode);
/* Execute a callback function if a specified AmphoraImage or AmphoraString is clicked, button is an SDL_BUTTON_X macro */
bool Amphora_ObjectClickedV1(void *spr, int button, void (*callback)(void));
/* Returns true if the mouse is over the provided object, false if not */
bool Amphora_ObjectHoverV1(void *obj);
/* Gets the currently pressed key */
SDL_Keycode Amphora_GetPressedKeyV1(void);
/* Returns whether the left joystick is currently in use */
bool Amphora_LeftJoystickActiveV1(void);
/* Returns whether the right joystick is currently in use */
bool Amphora_RightJoystickActiveV1(void);
/* Get the state of the left joystick */
Vector2f Amphora_GetLeftJoystickStateV1(void);
/* Get the state of the left joystick */
Vector2f Amphora_GetRightJoystickStateV1(void);
/* Get the name of the key associated with an action */
const char *Amphora_GetActionKeyNameV1(const char *action);
/* Run a callback function for each defined action name */
void Amphora_ForEachActionV1(void (*callback)(const char *, int));
#ifdef __cplusplus
}
#endif

#endif /* UNTITLED_PLATFORMER_INPUT_H */
