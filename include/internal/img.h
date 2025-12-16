#ifndef IMG_INTERNAL_H
#define IMG_INTERNAL_H

#include "../img.h"
#include "internal/ht_hash.h"
#include "internal/render.h"

struct frameset_t {
	SDL_Texture *override_img;
	SDL_Rect rectangle;
	Sint16 current_frame;
	Uint16 num_frames;
	Uint32 delay;
	Uint32 last_change;
	Vector2f position_offset;
	void (*callback)(void);
	bool playing_oneshot : 1;
};

struct sprite_t {
	enum amphora_object_type_e type;
	SDL_FRect rectangle;
	SDL_Texture *image;
	SDL_Surface *surface;
	SDL_Surface *surface_orig;
	float scale;
	bool transient : 1;
	bool flip : 1;
	bool rerender : 1;
	HT_HashTable framesets;
	int current_frameset;
	struct frameset_t *frameset_list;
	Sint32 num_framesets;
	struct render_list_node_t *render_list_node;
};

int Amphora_InitIMG(void);
void Amphora_FreeAllIMG(void);
void Amphora_CloseIMG(void);
SDL_Texture *Amphora_GetIMGTextureByName(const char *name);
void Amphora_UpdateAndDrawSprite(AmphoraImage *spr);

#endif /* IMG_INTERNAL_H */
