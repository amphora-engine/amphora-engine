#ifndef DISABLE_FONTS
#ifndef AMPHORA_TTF_H
#define AMPHORA_TTF_H

#include "SDL.h"
#include "SDL_ttf.h"

#include "render.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Create a string */
AmphoraString *Amphora_CreateString(const char *font_name, int pt, float x, float y, int order, AmphoraColor color, bool stationary, const char *fmt, va_list args);
/* Get the number of characters in a message */
size_t Amphora_GetStringLength(const AmphoraString *msg);
/* Get the number of characters currently displayed in a message */
size_t Amphora_GetNumCharactersDisplayed(const AmphoraString *msg);
/* Get the text of an AmphoraString */
const char *Amphora_GetStringText(AmphoraString *msg);
/* Get the character at a specified string index */
char Amphora_GetStringCharAtIndex(const AmphoraString *msg, int idx);
/* Update the text in a string */
AmphoraString *Amphora_UpdateStringText(AmphoraString *msg, const char *fmt, va_list args);
/* Change the number of characters displayed in a string, 0 displays all characters */
AmphoraString *Amphora_UpdateStringCharsDisplayed(AmphoraString *msg, size_t n);
/* Free a string */
void Amphora_FreeString(AmphoraString *msg);
/* Display an AmphoraString on the screen */
void Amphora_RenderString(const AmphoraString *msg);
#ifdef __cplusplus
}
#endif

#endif /* AMPHORA_TTF_H */
#endif
