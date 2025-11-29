#include "internal/error.h"
#include "internal/img.h"
#include "internal/lib.h"
#include "internal/memory.h"
#include "internal/render.h"

/* Prototypes for private functions */
static void Amphora_LoadIMGTexture(const char *name);

/* File-scoped variables */
static HT_HashTable images;
static HT_HashTable image_surfaces;
static HT_HashTable image_surfaces_orig;
static HT_HashTable open_images;
static const char **img_names;
static const char **img_paths;
static int img_count;

Vector2f
Amphora_GetSpritePosition(const AmphoraImage *spr)
{
	Amphora_ValidatePtrNotNull(spr, ((Vector2f){0, 0 }))

	return (Vector2f){ spr->rectangle.x, spr->rectangle.y };
}

Vector2f
Amphora_GetSpriteCenter(const AmphoraImage *spr)
{
	Amphora_ValidatePtrNotNull(spr, ((Vector2f){0, 0 }))

	return (Vector2f){
		.x = spr->rectangle.x + ((float)spr->frameset_list[spr->current_frameset].rectangle.w / 2) - spr->frameset_list[spr->current_frameset].position_offset.x,
		.y = spr->rectangle.y + ((float)spr->frameset_list[spr->current_frameset].rectangle.h / 2) - spr->frameset_list[spr->current_frameset].position_offset.y
	};
}

bool
Amphora_IsSpriteFlipped(const AmphoraImage *spr)
{
	Amphora_ValidatePtrNotNull(spr, false)

	return spr->flip;
}

AmphoraImage *
Amphora_CreateSprite(const char *image_name,
			const float x,
			const float y,
			const float scale,
			const bool flip,
			const bool stationary,
			const Sint32 order)
{
	AmphoraImage *spr = NULL;
	struct render_list_node_t *render_list_node = NULL;

	if (HT_GetValue(image_name, open_images) == -1)
		Amphora_LoadIMGTexture(image_name);

	if ((spr = Amphora_HeapCalloc(1, sizeof(AmphoraImage), MEM_IMAGE)) == NULL)
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to initialize sprite\n");

		return NULL;
	}
	render_list_node = Amphora_AddRenderListNode(order);

	spr->type = AMPH_OBJ_SPR;
	spr->image = HT_GetRef(image_name, SDL_Texture, open_images);
	spr->surface = HT_GetRef(image_name, SDL_Surface, image_surfaces);
	spr->surface_orig = HT_GetRef(image_name, SDL_Surface, image_surfaces_orig);
	spr->rectangle.x = x;
	spr->rectangle.y = y;
	spr->scale = scale;
	spr->framesets = HT_NewTable();
	spr->render_list_node = render_list_node;
	spr->flip = flip;
	render_list_node->type = AMPH_OBJ_SPR;
	render_list_node->data = spr;
	render_list_node->stationary = stationary;

	return spr;
}

int
Amphora_AddFrameset(AmphoraImage *spr,
			const char *name,
			const char *override_img,
			Sint32 sx,
			Sint32 sy,
			Sint32 w,
			Sint32 h,
			float off_x,
			float off_y,
			Uint16 num_frames,
			Uint16 delay)
{
	SDL_Texture *override = NULL;

	Amphora_ValidatePtrNotNull(spr, AMPHORA_STATUS_FAIL_UNDEFINED)
	if (!((spr->frameset_list = Amphora_HeapRealloc(spr->frameset_list, (spr->num_framesets + 1) * sizeof(struct frameset_t), MEM_IMAGE))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to reallocate framesets\n");

		return AMPHORA_STATUS_ALLOC_FAIL;
	}

	if (override_img)
	{
		if (HT_GetValue(override_img, open_images) == -1)
			Amphora_LoadIMGTexture(override_img);

		override = HT_GetRef(override_img, SDL_Texture, open_images);
	}

	spr->frameset_list[spr->num_framesets] = (struct frameset_t){
		.override_img = override,
		.rectangle.x = sx,
		.rectangle.y = sy,
		.rectangle.w = w,
		.rectangle.h = h,
		.current_frame = -1,
		.num_frames = num_frames,
		.delay = delay,
		.position_offset = (Vector2f){off_x, off_y }
	};
	(void)HT_SetValue(name, spr->num_framesets, spr->framesets);
	if (++spr->num_framesets == 1)
	{
		spr->current_frameset = 0;
		spr->rectangle.w = (float)spr->frameset_list[0].rectangle.w * spr->scale;
		spr->rectangle.h = (float)spr->frameset_list[0].rectangle.h * spr->scale;
	}

	return AMPHORA_STATUS_OK;
}

void
Amphora_SetFrameset(AmphoraImage *spr, const char *name)
{
	int idx = (int)HT_GetValue(name, spr->framesets);
	struct frameset_t *frameset = &spr->frameset_list[idx];

	if (idx == spr->current_frameset) return;

	frameset->playing_oneshot = false;
	spr->current_frameset = idx;
	spr->rectangle.w = (float)frameset->rectangle.w * spr->scale;
	spr->rectangle.h = (float)frameset->rectangle.h * spr->scale;
}

void
Amphora_PlayOneshot(AmphoraImage *spr, const char *name, void (*callback)(void))
{
	int idx = (int)HT_GetValue(name, spr->framesets);
	struct frameset_t *frameset = &spr->frameset_list[idx];

	if (idx == spr->current_frameset) return;

	frameset->playing_oneshot = true;
	spr->current_frameset = idx;
	spr->rectangle.w = (float)frameset->rectangle.w * spr->scale;
	spr->rectangle.h = (float)frameset->rectangle.h * spr->scale;
	frameset->current_frame = -1;
	frameset->last_change = SDL_GetTicks();
	frameset->callback = callback;
}

int
Amphora_SetFramesetAnimationTime(AmphoraImage *spr, const char *name, Uint16 delay)
{
	struct frameset_t *frameset = &spr->frameset_list[HT_GetValue(name, spr->framesets)];

	if (!frameset) return AMPHORA_STATUS_FAIL_UNDEFINED;

	frameset->delay = delay;

	return AMPHORA_STATUS_OK;
}

AmphoraImage *
Amphora_ReorderSprite(AmphoraImage *spr, Sint32 order)
{
	struct render_list_node_t *new_node = Amphora_AddRenderListNode(order);
	struct render_list_node_t *old_node = spr->render_list_node;

	if (new_node == NULL)
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate render list node, not reordering\n");

		return spr;
	}

	new_node->type = AMPH_OBJ_SPR;
	new_node->data = spr;
	new_node->display = old_node->display;
	new_node->stationary = old_node->stationary;
	old_node->garbage = true;
	spr->render_list_node = new_node;

	return spr;
}

int
Amphora_SetSpriteLocation(AmphoraImage *spr, float x, float y)
{
	Amphora_ValidatePtrNotNull(spr, AMPHORA_STATUS_FAIL_UNDEFINED)

	spr->rectangle.x = x;
	spr->rectangle.y = y;

	return AMPHORA_STATUS_OK;
}

int
Amphora_MoveSprite(AmphoraImage *spr, float delta_x, float delta_y)
{
	Amphora_ValidatePtrNotNull(spr, AMPHORA_STATUS_FAIL_UNDEFINED)

	spr->rectangle.x += delta_x;
	spr->rectangle.y += delta_y;

	return AMPHORA_STATUS_OK;
}

int
Amphora_FlipSprite(AmphoraImage *spr)
{
	Amphora_ValidatePtrNotNull(spr, AMPHORA_STATUS_FAIL_UNDEFINED)

	spr->flip = true;

	return AMPHORA_STATUS_OK;
}

int
Amphora_UnflipSprite(AmphoraImage *spr)
{
	Amphora_ValidatePtrNotNull(spr, AMPHORA_STATUS_FAIL_UNDEFINED)

	spr->flip = false;

	return AMPHORA_STATUS_OK;
}

int
Amphora_ShowSprite(AmphoraImage *spr)
{
	Amphora_ValidatePtrNotNull(spr, AMPHORA_STATUS_FAIL_UNDEFINED)

	spr->render_list_node->display = true;

	return AMPHORA_STATUS_OK;
}

int
Amphora_HideSprite(AmphoraImage *spr)
{
	Amphora_ValidatePtrNotNull(spr, AMPHORA_STATUS_FAIL_UNDEFINED)

	spr->render_list_node->display = false;

	return AMPHORA_STATUS_OK;
}

void
Amphora_ApplyFXToImage(AmphoraImage *img, void (*fx)(SDL_Surface *))
{
	if (!fx) return;

	fx(img->surface);
	img->rerender = true;
}

void
Amphora_ResetImage(AmphoraImage *img)
{
	(void)SDL_memcpy(img->surface->pixels, img->surface_orig->pixels,
		img->surface_orig->w * img->surface_orig->h * img->surface_orig->format->BytesPerPixel);
	img->rerender = true;
}

int
Amphora_FreeSprite(AmphoraImage *spr)
{
	Amphora_ValidatePtrNotNull(spr, AMPHORA_STATUS_FAIL_UNDEFINED)

	if (spr == Amphora_GetCameraTarget()) Amphora_SetCameraTarget(NULL);
	if (spr->frameset_list) Amphora_HeapFree(spr->frameset_list);
	HT_FreeTable(spr->framesets);
	spr->render_list_node->garbage = true;
	Amphora_HeapFree(spr);

	return AMPHORA_STATUS_OK;
}

/*
 * Internal functions
 */

int
Amphora_InitIMG(void)
{
	int i;

	images = HT_NewTable();
	open_images = HT_NewTable();
	image_surfaces = HT_NewTable();
	image_surfaces_orig = HT_NewTable();

	for (i = 0; i < img_count; i++)
	{
		HT_StoreRef(img_names[i], img_paths[i], images);
#ifdef DEBUG
		SDL_Log("Found image %s\n", img_names[i]);
#endif
	}

	return 0;
}

void
Amphora_FreeAllIMG(void)
{
	int i;

	for (i = 0; i < img_count; i++)
	{
		if (HT_GetValue(img_names[i], open_images) != -1)
		{
#ifdef DEBUG
			SDL_Log("Unloading image: %s\n", img_names[i]);
#endif
			SDL_DestroyTexture(HT_GetRef(img_names[i], SDL_Texture, open_images));
			SDL_FreeSurface(HT_GetRef(img_names[i], SDL_Surface, image_surfaces));
			SDL_FreeSurface(HT_GetRef(img_names[i], SDL_Surface, image_surfaces_orig));
			(void)HT_SetValue(img_names[i], 0, open_images);
			(void)HT_SetValue(img_names[i], 0, image_surfaces);
			(void)HT_SetValue(img_names[i], 0, image_surfaces_orig);
			HT_DeleteKey(img_names[i], open_images);
			HT_DeleteKey(img_names[i], image_surfaces);
			HT_DeleteKey(img_names[i], image_surfaces_orig);
		}
	}
}

void
Amphora_CloseIMG(void)
{
	HT_FreeTable(images);
	HT_FreeTable(open_images);
	HT_FreeTable(image_surfaces);
	HT_FreeTable(image_surfaces_orig);
}

SDL_Texture *
Amphora_GetIMGTextureByName(const char *name)
{
	if (HT_GetValue(name, open_images) == -1)
		Amphora_LoadIMGTexture(name);

	return HT_GetRef(name, SDL_Texture, open_images);
}

void
Amphora_UpdateAndDrawSprite(AmphoraImage *spr)
{
	struct frameset_t *frameset = &spr->frameset_list[spr->current_frameset];
	SDL_Rect src;
	SDL_FRect dst;
	const Vector2f camera = Amphora_GetCamera();
	Vector2 logical_size = Amphora_GetRenderLogicalSize();
	Uint32 cur_ms = SDL_GetTicks();

	if (!(spr->render_list_node->display && spr->num_framesets > 0)) return;

	if (spr->rerender)
	{
		if (SDL_UpdateTexture(spr->image, NULL, spr->surface->pixels, spr->surface->pitch) < 0)
			Amphora_SetError(AMPHORA_STATUS_FAIL_UNDEFINED, "SDL_UpdateTexture failed: %s", SDL_GetError());

		spr->rerender = false;
	}
	if (cur_ms - frameset->last_change > frameset->delay)
	{
		if (++frameset->current_frame == frameset->num_frames)
		{
			if (frameset->playing_oneshot)
			{
				frameset->current_frame--;
				if (frameset->callback)
					frameset->callback();
			}
			else
			{
				frameset->current_frame = 0;
			}
		}
		frameset->last_change = cur_ms;
	}
	if (frameset->current_frame == -1)
		frameset->current_frame = 0;

	src = frameset->rectangle;
	src.x += (src.w * frameset->current_frame);
	if (spr->render_list_node->stationary)
	{
		dst = (SDL_FRect){
			.x = spr->rectangle.x > 0 ? spr->rectangle.x : (float) Amphora_GetResolution().x + spr->rectangle.x - (float)frameset->rectangle.w,
			.y = spr->rectangle.y > 0 ? spr->rectangle.y : (float) Amphora_GetResolution().y + spr->rectangle.y - (float)frameset->rectangle.h,
			.w = (float)frameset->rectangle.w * spr->scale,
			.h = (float)frameset->rectangle.h * spr->scale
		};
	}
	else
	{
		dst = (SDL_FRect){
			.x = spr->rectangle.x - frameset->position_offset.x - camera.x,
			.y = spr->rectangle.y - frameset->position_offset.y - camera.y,
			.w = (float)frameset->rectangle.w * spr->scale,
			.h = (float)frameset->rectangle.h * spr->scale
		};
	}

	if (spr->render_list_node->stationary)
		Amphora_SetRenderLogicalSize(Amphora_GetResolution());

	Amphora_RenderTexture(frameset->override_img ? frameset->override_img : spr->image,
			      &src, &dst, 0, spr->flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	if (spr->render_list_node->stationary)
		Amphora_SetRenderLogicalSize(logical_size);
}

/*
 * Private Functions
 */

static void
Amphora_LoadIMGTexture(const char *name)
{
	SDL_RWops *img_rw = NULL;
	SDL_Surface *surface, *surface_orig;

#ifdef DEBUG
	SDL_Log("Loading image: %s\n", name);
#endif
	img_rw = SDL_RWFromFile(HT_GetRef(name, char, images), "rb");
	surface = IMG_Load_RW(img_rw, 1);
	surface_orig = SDL_DuplicateSurface(surface);
	HT_StoreRef(name, SDL_CreateTextureFromSurface(Amphora_GetRenderer(), surface), open_images);
	HT_StoreRef(name, surface, image_surfaces);
	HT_StoreRef(name, surface_orig, image_surfaces_orig);
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterImageData(const char **names, const char **paths, int count)
{
	img_names = names;
	img_paths = paths;
	img_count = count;
}
