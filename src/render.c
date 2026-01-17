#include "internal/context.h"
#include "internal/error.h"
#include "internal/img.h"
#include "internal/memory.h"
#include "internal/particles.h"
#include "internal/prefs.h"
#include "internal/render.h"
#include "internal/tilemap.h"
#include "internal/ttf.h"

/* Prototypes for private functions */
int Amphora_InitRenderList(void);

static struct render_ctx init =
{
	.camera = { 0, 0 },
	.camera_mode = CAM_MANUAL,
	.bg = { 0, 0, 0, 0xff },
	.render_logical_size = { 0, 0 }
};
static struct render_ctx *inst = &init;

Vector2
Amphora_GetResolutionV1(void)
{
	Sint32 rx, ry;
	SDL_GetWindowSize(inst->window, &rx, &ry);
	return (Vector2){rx, ry };
}

Vector2
Amphora_GetRenderLogicalSizeV1(void)
{
	return inst->render_logical_size;
}

Vector2f
Amphora_GetCameraV1(void)
{
	return inst->camera;
}

void
Amphora_SetCameraV1(float x, float y)
{
	inst->camera.x = x;
	inst->camera.y = y;
}

void
Amphora_MoveCameraV1(float delta_x, float delta_y)
{
	inst->camera.x += delta_x;
	inst->camera.y += delta_y;
}

void
Amphora_SetCameraTargetV1(AmphoraImage *target)
{
	inst->camera_mode = target ? CAM_TRACKING : CAM_MANUAL;
	inst->camera_target = target;
}

void
Amphora_BoundCameraToMapV1(void)
{
	AmphoraFRect map_rect = *Amphora_GetMapRectangleV1();
	SDL_FRect rect = { map_rect.x, map_rect.y, map_rect.w, map_rect.h };

	SDL_memcpy(&inst->camera_boundary, &rect, sizeof(inst->camera_boundary));
}

void
Amphora_BoundCameraV1(const AmphoraFRect *boundary)
{
	SDL_FRect rect = { boundary->x, boundary->y, boundary->w, boundary->h };

	SDL_memcpy(&inst->camera_boundary, &rect, sizeof(inst->camera_boundary));
}

void
Amphora_UnboundCameraV1(void)
{
	SDL_memset(&inst->camera_boundary, 0, sizeof(inst->camera_boundary));
}

void
Amphora_SetCameraZoomV1(int factor, int delay)
{
	static int current_factor = 100;
	static Vector2 *scale_steps = NULL;
	static int scale_steps_index = 0;
	static int scale_steps_count = 0;
	Vector2 current_resolution = Amphora_GetResolutionV1();
	Vector2 current_logical_size = Amphora_GetRenderLogicalSizeV1();
	int i;
	Vector2 step_size = {
		.x = (current_logical_size.x - ((current_resolution.x * 100) / factor)) / (delay ? delay : 1),
		.y = (current_logical_size.y - ((current_resolution.y * 100) / factor)) / (delay ? delay : 1)
	};

	if (factor <= 0) return;
	if (delay < 0) delay = 0;

	if (!scale_steps && delay > 0)
	{
		scale_steps_count = delay;
		if (factor == current_factor) return;

		current_factor = factor;
		if (!((scale_steps = Amphora_HeapAlloc(delay * sizeof(Vector2), MEM_MISC))))
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate scale steps\n");
			return;
		}
		for (i = 0; i < scale_steps_count; i++)
		{
			scale_steps[i] = (Vector2){
				.x = inst->render_logical_size.x - (step_size.x * (i + 1)),
				.y = inst->render_logical_size.y - (step_size.y * (i + 1))
			};
		}
	}
	if (scale_steps_index == scale_steps_count || factor != current_factor)
	{
		Amphora_HeapFree(scale_steps);
		scale_steps = NULL;
		scale_steps_index = 0;
		scale_steps_count = 0;
		if (current_factor == 100)
			Amphora_SetRenderLogicalSize(current_resolution);
#ifdef DEBUG
		SDL_Log("Finished scaling to %d, %d\n", Amphora_GetRenderLogicalSizeV1().x, Amphora_GetRenderLogicalSizeV1().y);
#endif
		return;
	}
	Amphora_SetRenderLogicalSize(scale_steps[scale_steps_index++]);
}

void
Amphora_ResetCameraZoomV1(int delay)
{
	Amphora_SetCameraZoomV1(100, delay);
}

AmphoraColor
Amphora_GetBGColorV1(void)
{
	AmphoraColor abg = { .r = inst->bg.r, .g = inst->bg.g, .b = inst->bg.b, .a = inst->bg.a};

	return abg;
}

void
Amphora_SetBGColorV1(AmphoraColor color)
{
	inst->bg.r = color.r;
	inst->bg.g = color.g;
	inst->bg.b = color.b;
	inst->bg.a = 0xff;
}

void
Amphora_SetWindowFullscreenV1(void)
{
	SDL_SetWindowFullscreen(inst->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void
Amphora_SetWindowWindowedV1(void)
{
	SDL_SetWindowFullscreen(inst->window, 0);
}

bool
Amphora_IsWindowFullscreenV1(void)
{
	return SDL_GetWindowFlags(inst->window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
}

/*
 * Internal functions
 */

int
Amphora_InitRender(void)
{
	if (!((init.window = SDL_CreateWindow(init.window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
					 (int) Amphora_LoadWinX(), (int) Amphora_LoadWinY(), (Uint32) Amphora_LoadWinFlags()))))
	{
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to create window: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to create window", SDL_GetError(), 0);
		return -1;
	}
	if (!((init.renderer = SDL_CreateRenderer(init.window, -1, 0))))
	{
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to create renderer: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to create renderer", SDL_GetError(), 0);
		return -1;
	}
	if (Amphora_InitIMG() == -1)
	{
		SDL_LogError(SDL_LOG_CATEGORY_RENDER,"Failed to init image system\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to init image system", "Failed to initialize image system", 0);
		return -1;
	}
	Amphora_SetRenderLogicalSize(Amphora_GetResolutionV1());

	return 0;
}

void
Amphora_CloseRender(void)
{
	SDL_DestroyRenderer(init.renderer);
	SDL_DestroyWindow(init.window);
}

void
Amphora_SetRenderLogicalSize(Vector2 size)
{
	inst->render_logical_size = size;
	SDL_RenderSetLogicalSize(inst->renderer, (int)inst->render_logical_size.x, (int)inst->render_logical_size.y);
}

void
Amphora_ClearBG(void)
{
	SDL_SetRenderDrawColor(inst->renderer, inst->bg.r, inst->bg.g, inst->bg.b, inst->bg.a);
	SDL_RenderClear(inst->renderer);
}

SDL_Window *
Amphora_GetWindow(void)
{
	return inst->window;
}

SDL_Renderer *
Amphora_GetRenderer(void)
{
	return inst->renderer;
}

AmphoraImage *
Amphora_GetCameraTarget(void)
{
	return inst->camera_target;
}

struct render_list_node_t *
Amphora_AddRenderListNode(int order)
{
	struct render_list_node_t *new_render_list_node = NULL;

	if (!inst->render_list) Amphora_InitRenderList();

	if ((new_render_list_node = Amphora_HeapCalloc(1, sizeof(struct render_list_node_t), MEM_RENDERABLE)) == NULL)
	{
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "Failed to initialize new render list node\n");

		return NULL;
	}
	while (1)
	{
		if (inst->render_list->next == NULL)
		{
			new_render_list_node->next = NULL;
			inst->render_list->next = new_render_list_node;
			break;
		}
		if (inst->render_list->next->order > order)
		{
			new_render_list_node->next = inst->render_list->next;
			inst->render_list->next = new_render_list_node;
			break;
		}
		inst->render_list = inst->render_list->next;
	}
	new_render_list_node->order = order;
	new_render_list_node->display = true;
	inst->render_list = inst->render_list_head;
	inst->render_list_node_count++;

	return new_render_list_node;
}

void
Amphora_ProcessRenderList(void)
{
	struct render_list_node_t *garbage;
	AmphoraFRect map_rect;
	SDL_FRect rect;

	while(inst->render_list)
	{
		while (inst->render_list->next && inst->render_list->next->garbage)
		{
			garbage = inst->render_list->next;
			inst->render_list->next = inst->render_list->next->next;
			Amphora_HeapFree(garbage);
			inst->render_list_node_count--;
		}
		if (!inst->render_list->display)
		{
			inst->render_list = inst->render_list->next;
			continue;
		}
		switch (inst->render_list->type)
		{
			case AMPH_OBJ_SPR:
				Amphora_UpdateAndDrawSprite(inst->render_list->data);
				break;
			case AMPH_OBJ_TXT:
				Amphora_RenderStringV1(inst->render_list->data);
				break;
			case AMPH_OBJ_MAP:
				map_rect = *Amphora_GetMapRectangleV1();
				map_rect.x = -inst->camera.x;
				map_rect.y = -inst->camera.y;
				rect = (SDL_FRect){ map_rect.x, map_rect.y, map_rect.w, map_rect.h };
				Amphora_RenderTexture(inst->render_list->data, NULL, &rect, 0,
						      SDL_FLIP_NONE);
				break;
			case AMPH_OBJ_EMITTER:
				Amphora_UpdateAndRenderParticleEmitter(inst->render_list->data);
				break;
			default:
				break;
		}

		inst->render_list = inst->render_list->next;
	}

	inst->render_list = inst->render_list_head;
}

void
Amphora_FreeRenderList(void)
{
	struct render_list_node_t **allocated_addrs = Amphora_HeapAlloc(inst->render_list_node_count * sizeof(struct render_list_node_t *), MEM_MISC);
	Uint32 i = 0;

	while (inst->render_list)
	{
		allocated_addrs[i++] = inst->render_list;
		inst->render_list = inst->render_list->next;
	}
	for (i = 0; i < inst->render_list_node_count; i++)
	{
		if (allocated_addrs[i]->garbage)
		{
			Amphora_HeapFree(allocated_addrs[i]);

			continue;
		}

		switch(allocated_addrs[i]->type)
		{
			case AMPH_OBJ_SPR:
				Amphora_FreeSpriteV1(allocated_addrs[i]->data);
				break;
			case AMPH_OBJ_TXT:
				Amphora_FreeStringV1(allocated_addrs[i]->data);
				break;
			case AMPH_OBJ_MAP:
				SDL_DestroyTexture(allocated_addrs[i]->data);
				break;
			case AMPH_OBJ_EMITTER:
				Amphora_DestroyEmitterV1(allocated_addrs[i]->data);
			default:
				break;
		}
		Amphora_HeapFree(allocated_addrs[i]);
	}
	Amphora_HeapFree(allocated_addrs);
}

void
Amphora_ResetRenderList(void)
{
	Amphora_FreeRenderList();
	Amphora_InitRenderList();
}

void
Amphora_UpdateCamera(void)
{
	if (inst->camera_mode == CAM_MANUAL) return;

	inst->camera = Amphora_GetSpriteCenterV1(inst->camera_target);
	inst->camera.x -= (float)inst->render_logical_size.x / 2.0f;
	inst->camera.y -= (float)inst->render_logical_size.y / 2.0f;
	if (!inst->camera_boundary.w && !inst->camera_boundary.h) return;

	if (inst->camera.x < inst->camera_boundary.x || inst->camera.x + (float)inst->render_logical_size.x > inst->camera_boundary.x + inst->camera_boundary.w)
	{
		inst->camera.x = inst->camera.x > inst->camera_boundary.x ?
			inst->camera_boundary.x + inst->camera_boundary.w - (float)inst->render_logical_size.x :
			inst->camera_boundary.x;
	}
	if (inst->camera.y < inst->camera_boundary.y || inst->camera.y + (float)inst->render_logical_size.y > inst->camera_boundary.y + inst->camera_boundary.h)
	{
		inst->camera.y = inst->camera.y > inst->camera_boundary.y ?
			inst->camera_boundary.y + inst->camera_boundary.h - (float)inst->render_logical_size.y :
			inst->camera_boundary.y;
	}
}

void
Amphora_RenderTexture(SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect, double angle, SDL_RendererFlip flip)
{
	SDL_RenderCopyExF(inst->renderer, texture, srcrect, dstrect, angle, NULL, flip);
}

/*
 * Private functions
 */

int
Amphora_InitRenderList(void)
{
	if ((init.render_list = Amphora_HeapAlloc(sizeof(struct render_list_node_t), MEM_RENDERABLE)) == NULL)
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to initialize render list\n");

		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	init.render_list->type = AMPH_OBJ_NIL;
	init.render_list->order = SDL_MIN_SINT32;
	init.render_list->garbage = false;
	init.render_list->next = NULL;
	init.render_list_head = init.render_list;
	init.render_list_node_count = 1;

	return AMPHORA_STATUS_OK;
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterWindowTitle(const char *title)
{
	init.window_title = title;
}
