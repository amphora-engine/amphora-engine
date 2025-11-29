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

/* File-scoped variables */
static SDL_Renderer *renderer;
static SDL_Window *window;
static const char *window_title;
static Camera camera = { 0, 0 };
static enum camera_mode_e camera_mode = CAM_MANUAL;
static SDL_Color bg = { 0, 0, 0, 0xff };
static Vector2 render_logical_size = {0, 0 };
static struct render_list_node_t *render_list;
static struct render_list_node_t *render_list_head;
static AmphoraImage *camera_target;
static SDL_FRect camera_boundary;
static Uint32 render_list_node_count;

Vector2
Amphora_GetResolution(void)
{
	Sint32 rx, ry;
	SDL_GetWindowSize(window, &rx, &ry);
	return (Vector2){rx, ry };
}

Vector2
Amphora_GetRenderLogicalSize(void)
{
	return render_logical_size;
}

Vector2f
Amphora_GetCamera(void)
{
	return camera;
}

void
Amphora_SetCamera(float x, float y)
{
	camera.x = x;
	camera.y = y;
}

void
Amphora_MoveCamera(float delta_x, float delta_y)
{
	camera.x += delta_x;
	camera.y += delta_y;
}

void
Amphora_SetCameraTarget(AmphoraImage *target)
{
	camera_mode = target ? CAM_TRACKING : CAM_MANUAL;
	camera_target = target;
}

void
Amphora_BoundCameraToMap(void)
{
	SDL_memcpy(&camera_boundary, Amphora_GetMapRectangle(), sizeof(camera_boundary));
}

void
Amphora_BoundCamera(const SDL_FRect *boundary)
{
	SDL_memcpy(&camera_boundary, boundary, sizeof(camera_boundary));
}

void
Amphora_UnboundCamera(void)
{
	SDL_memset(&camera_boundary, 0, sizeof(camera_boundary));
}

void
Amphora_SetCameraZoom(Uint16 factor, Uint16 delay)
{
	static Uint16 current_factor = 100;
	static Vector2 *scale_steps = NULL;
	static Uint16 scale_steps_index = 0;
	static Uint16 scale_steps_count = 0;
	Vector2 current_resolution = Amphora_GetResolution();
	Vector2 current_logical_size = Amphora_GetRenderLogicalSize();
	int i;
	Vector2 step_size = {
		.x = (current_logical_size.x - ((current_resolution.x * 100) / factor)) / (delay ? delay : 1),
		.y = (current_logical_size.y - ((current_resolution.y * 100) / factor)) / (delay ? delay : 1)
	};

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
				.x = render_logical_size.x - (step_size.x * (i + 1)),
				.y = render_logical_size.y - (step_size.y * (i + 1))
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
		SDL_Log("Finished scaling to %d, %d\n", Amphora_GetRenderLogicalSize().x, Amphora_GetRenderLogicalSize().y);
#endif
		return;
	}
	Amphora_SetRenderLogicalSize(scale_steps[scale_steps_index++]);
}

void
Amphora_ResetCameraZoom(Uint16 delay)
{
	Amphora_SetCameraZoom(100, delay);
}

SDL_Color
Amphora_GetBGColor(void)
{
	return bg;
}

void
Amphora_SetBGColor(SDL_Color color)
{
	bg = color;
}

void
Amphora_SetWindowFullscreen(void)
{
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void
Amphora_SetWindowWindowed(void)
{
	SDL_SetWindowFullscreen(window, 0);
}

bool
Amphora_IsWindowFullscreen(void)
{
	return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
}

/*
 * Internal functions
 */

int
Amphora_InitRender(void)
{
	if (!((window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
					 (int) Amphora_LoadWinX(), (int) Amphora_LoadWinY(), (Uint32) Amphora_LoadWinFlags()))))
	{
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to create window: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to create window", SDL_GetError(), 0);
		return -1;
	}
	if (!((renderer = SDL_CreateRenderer(window, -1, 0))))
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
	Amphora_SetRenderLogicalSize(Amphora_GetResolution());

	return 0;
}

void
Amphora_CloseRender(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void
Amphora_SetRenderLogicalSize(Vector2 size)
{
	render_logical_size = size;
	SDL_RenderSetLogicalSize(renderer, (int)render_logical_size.x, (int)render_logical_size.y);
}

void
Amphora_ClearBG(void)
{
	SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
	SDL_RenderClear(renderer);
}

SDL_Window *
Amphora_GetWindow(void)
{
	return window;
}

SDL_Renderer *
Amphora_GetRenderer(void)
{
	return renderer;
}

AmphoraImage *
Amphora_GetCameraTarget(void)
{
	return camera_target;
}

struct render_list_node_t *
Amphora_AddRenderListNode(int order)
{
	struct render_list_node_t *new_render_list_node = NULL;

	if (!render_list) Amphora_InitRenderList();

	if ((new_render_list_node = Amphora_HeapCalloc(1, sizeof(struct render_list_node_t), MEM_RENDERABLE)) == NULL)
	{
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "Failed to initialize new render list node\n");

		return NULL;
	}
	while (1)
	{
		if (render_list->next == NULL)
		{
			new_render_list_node->next = NULL;
			render_list->next = new_render_list_node;
			break;
		}
		if (render_list->next->order > order)
		{
			new_render_list_node->next = render_list->next;
			render_list->next = new_render_list_node;
			break;
		}
		render_list = render_list->next;
	}
	new_render_list_node->order = order;
	new_render_list_node->display = true;
	render_list = render_list_head;
	render_list_node_count++;

	return new_render_list_node;
}

void
Amphora_ProcessRenderList(void)
{
	struct render_list_node_t *garbage;
	SDL_FRect map_rect;

	while(render_list)
	{
		while (render_list->next && render_list->next->garbage)
		{
			garbage = render_list->next;
			render_list->next = render_list->next->next;
			Amphora_HeapFree(garbage);
			render_list_node_count--;
		}
		if (!render_list->display)
		{
			render_list = render_list->next;
			continue;
		}
		switch (render_list->type)
		{
			case AMPH_OBJ_SPR:
				Amphora_UpdateAndDrawSprite(render_list->data);
				break;
			case AMPH_OBJ_TXT:
				Amphora_RenderString(render_list->data);
				break;
			case AMPH_OBJ_MAP:
				map_rect = *Amphora_GetMapRectangle();
				map_rect.x = -camera.x;
				map_rect.y = -camera.y;
				Amphora_RenderTexture(render_list->data, NULL, &map_rect, 0,
						      SDL_FLIP_NONE);
				break;
			case AMPH_OBJ_EMITTER:
				Amphora_UpdateAndRenderParticleEmitter(render_list->data);
				break;
			default:
				break;
		}

		render_list = render_list->next;
	}

	render_list = render_list_head;
}

void
Amphora_FreeRenderList(void)
{
	struct render_list_node_t **allocated_addrs = Amphora_HeapAlloc(render_list_node_count * sizeof(struct render_list_node_t *), MEM_MISC);
	Uint32 i = 0;

	while (render_list)
	{
		allocated_addrs[i++] = render_list;
		render_list = render_list->next;
	}
	for (i = 0; i < render_list_node_count; i++)
	{
		switch(allocated_addrs[i]->type)
		{
			case AMPH_OBJ_SPR:
				Amphora_FreeSprite((AmphoraImage *)allocated_addrs[i]->data);
				break;
			case AMPH_OBJ_TXT:
				Amphora_FreeString((AmphoraString *)allocated_addrs[i]->data);
				break;
			case AMPH_OBJ_MAP:
				SDL_DestroyTexture((SDL_Texture *)allocated_addrs[i]->data);
				break;
			case AMPH_OBJ_EMITTER:
				Amphora_DestroyEmitter((AmphoraEmitter *)allocated_addrs[i]->data);
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
	if (camera_mode == CAM_MANUAL) return;

	camera = Amphora_GetSpriteCenter(camera_target);
	camera.x -= (float)render_logical_size.x / 2.0f;
	camera.y -= (float)render_logical_size.y / 2.0f;
	if (!camera_boundary.w && !camera_boundary.h) return;

	if (camera.x < camera_boundary.x || camera.x + (float)render_logical_size.x > camera_boundary.x + camera_boundary.w)
	{
		camera.x = camera.x > camera_boundary.x ?
			camera_boundary.x + camera_boundary.w - (float)render_logical_size.x :
			camera_boundary.x;
	}
	if (camera.y < camera_boundary.y || camera.y + (float)render_logical_size.y > camera_boundary.y + camera_boundary.h)
	{
		camera.y = camera.y > camera_boundary.y ?
			camera_boundary.y + camera_boundary.h - (float)render_logical_size.y :
			camera_boundary.y;
	}
}

void
Amphora_RenderTexture(SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect, double angle, SDL_RendererFlip flip)
{
	SDL_RenderCopyExF(renderer, texture, srcrect, dstrect, angle, NULL, flip);
}

/*
 * Private functions
 */

int
Amphora_InitRenderList(void)
{
	if ((render_list = Amphora_HeapAlloc(sizeof(struct render_list_node_t), MEM_RENDERABLE)) == NULL)
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to initialize render list\n");

		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	render_list->type = AMPH_OBJ_NIL;
	render_list->order = SDL_MIN_SINT32;
	render_list->garbage = false;
	render_list->next = NULL;
	render_list_head = render_list;
	render_list_node_count = 1;

	return AMPHORA_STATUS_OK;
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterWindowTitle(const char *title)
{
	window_title = title;
}
