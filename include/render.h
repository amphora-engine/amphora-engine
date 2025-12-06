#ifndef UNTITLED_PLATFORMER_RENDER_H
#define UNTITLED_PLATFORMER_RENDER_H

#include "SDL.h"

#include "util.h"

typedef SDL_FPoint Vector2f;
typedef SDL_FPoint Camera;
typedef SDL_Point Vector2;
typedef struct sprite_t AmphoraImage;
typedef struct amphora_message_t AmphoraString;
typedef struct emitter_t AmphoraEmitter;

typedef struct amphora_frect_t
{
	float x, y, w, h;
} AmphoraFRect;

typedef struct amphora_color_t
{
	uint8_t r, g, b, a;
} AmphoraColor;

typedef union amphora_pixeldata_abgr_u
{
	AmphoraColor color_data;
	Uint32 pixel;
} AmphoraPixelDataABGR;

typedef struct amphora_surface_t
{
	AmphoraPixelDataABGR *pixels;
	int w, h;
} AmphoraSurface;

#ifdef __cplusplus
extern "C" {
#endif
/* Get the resolution */
Vector2 Amphora_GetResolution(void);
/* Get the renderer logical size */
Vector2 Amphora_GetRenderLogicalSize(void);
/* Get the current upper-left point of the camera */
Vector2f Amphora_GetCamera(void);
/* Set the location of the camera's top left point */
void Amphora_SetCamera(float x, float y);
/* Move the camera */
void Amphora_MoveCamera(float delta_x, float delta_y);
/* Set the sprite for the camera to follow, NULL to disable tracking */
void Amphora_SetCameraTarget(AmphoraImage *target);
/* Bound the camera to the current map area */
void Amphora_BoundCameraToMap(void);
/* Bound the camera to the supplied FRect */
void Amphora_BoundCamera(const AmphoraFRect *boundary);
/* Remove the currently set camera boundary */
void Amphora_UnboundCamera(void);
/* Set the zoom factor for the camera */
void Amphora_SetCameraZoom(int factor, int delay);
/* Reset the camera zoom */
void Amphora_ResetCameraZoom(int delay);
/* Gets the current background color */
AmphoraColor Amphora_GetBGColor(void);
/* Sets the background color */
void Amphora_SetBGColor(AmphoraColor color);
/* Sets the game window to fullscreen */
void Amphora_SetWindowFullscreen(void);
/* Sets the game window to windowed */
void Amphora_SetWindowWindowed(void);
/* Returns true if the game window is fullscreen, false if windowed */
bool Amphora_IsWindowFullscreen(void);
#ifdef __cplusplus
}
#endif

#endif /* UNTITLED_PLATFORMER_RENDER_H */
