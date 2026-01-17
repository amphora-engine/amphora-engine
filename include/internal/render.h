#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H

#include <stdbool.h>

#include "../util.h"
#include "../render.h"

enum camera_mode_e {
	CAM_MANUAL,
	CAM_TRACKING
};

enum amphora_object_type_e {
	AMPH_OBJ_SPR,
	AMPH_OBJ_TXT,
	AMPH_OBJ_MAP,
	AMPH_OBJ_EMITTER,
	AMPH_OBJ_NIL
};

struct render_list_node_t {
	enum amphora_object_type_e type;
	void *data;
	int order;
	bool garbage : 1;
	bool display : 1;
	bool stationary : 1;
	struct render_list_node_t *next;
};

typedef struct {
	enum amphora_object_type_e type;
} IAmphoraObject;

int Amphora_InitRender(void);
void Amphora_CloseRender(void);
void Amphora_SetRenderLogicalSize(Vector2 size);
void Amphora_ClearBG(void);
SDL_Window *Amphora_GetWindow(void);
SDL_Renderer *Amphora_GetRenderer(void);
AmphoraImage *Amphora_GetCameraTarget(void);
struct render_list_node_t *Amphora_AddRenderListNode(int order);
void Amphora_ProcessRenderList(void);
void Amphora_FreeRenderList(void);
void Amphora_ResetRenderList(void);
void Amphora_UpdateCamera(void);
void Amphora_RenderTexture(SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect, double angle, SDL_RendererFlip flip);

#endif /* RENDER_INTERNAL_H */
