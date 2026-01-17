#include "internal/context.h"
#include "internal/error.h"
#include "internal/events.h"
#include "internal/ht_hash.h"
#include "internal/img.h"
#include "internal/lib.h"
#include "internal/memory.h"
#include "internal/render.h"
#include "internal/tilemap.h"

#define CUTE_TILED_IMPLEMENTATION
#include "vendor/cute_tiled.h"

/* Prototypes for private functions */
static int Amphora_ParseMapToTexture(const char *name);
static int Amphora_ParseTileLayer(const cute_tiled_map_t *map, const cute_tiled_layer_t *layer, int tileset_img_w, SDL_Texture *tileset_img, int n);
static int Amphora_ParseObjectGroup(const cute_tiled_layer_t *layer);
static int Amphora_GetMapLayerByName(const char *name);
static void Amphora_MapLayerFadeEvent(void);

static struct tilemap_ctx init;
static struct tilemap_ctx *inst = &init;

void
Amphora_SetMapV1(const char *name, const float scale)
{
	int i;
	struct render_list_node_t *render_list_node;

	Amphora_DestroyCurrentMap();
	if (!name) return;

	inst->current_map.scale = scale ? scale : 1;
	if (Amphora_ParseMapToTexture(name) == -1)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create texture from map: %s\n", name);
		return;
	}
	for (i = 0; i < inst->current_map.num_layers; i++)
	{
		render_list_node = Amphora_AddRenderListNode(100 * i);
		render_list_node->type = AMPH_OBJ_MAP;
		render_list_node->data = inst->current_map.layers[i].texture;
		inst->current_map.layers[i].node = render_list_node;
	}
}

const AmphoraFRect *
Amphora_GetMapRectangleV1(void)
{
	return &inst->map_rect;
}

int
Amphora_HideMapLayerV1(const char *name, int t)
{
	int i, n = Amphora_GetMapLayerByName(name);

	if (n == -1 || !inst->current_map.layers[n].node->display || inst->tilemap_flags.transitioning)
		return AMPHORA_STATUS_OK;

	inst->fade_layer = &inst->current_map.layers[n];
	if (t < 1)
	{
		inst->current_map.layers[n].node->display = false;
		return AMPHORA_STATUS_OK;
	}
	inst->transition_fader.timer = t;
	inst->transition_fader.frames = inst->transition_fader.timer * Amphora_GetFPSV1() / 1000;
	inst->transition_fader.idx = 0;
	if (!((inst->transition_fader.steps = Amphora_HeapAlloc(inst->transition_fader.frames * sizeof(Uint8), MEM_MISC))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate memory for fade steps\n");
		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	for (i = 0; i < inst->transition_fader.frames; i++)
	{
		inst->transition_fader.steps[i] = 255 - i * 255 / (inst->transition_fader.frames - 1);
	}
	inst->tilemap_flags.transitioning = true;
	inst->tilemap_flags.persist_shown = false;
	(void)Amphora_RegisterEventV1("amph_internal_map_layer_fade", Amphora_MapLayerFadeEvent);

	return AMPHORA_STATUS_OK;
}

int
Amphora_ShowMapLayerV1(const char *name, int t)
{
	int n = Amphora_GetMapLayerByName(name);
	int i;

	if (n == -1 || inst->current_map.layers[n].node->display || inst->tilemap_flags.transitioning)
		return AMPHORA_STATUS_OK;

	inst->current_map.layers[n].node->display = true;
	inst->fade_layer = &inst->current_map.layers[n];
	if (t < 1)
	{
		(void)SDL_SetTextureAlphaMod(inst->fade_layer->texture, 0xff);
		return AMPHORA_STATUS_OK;
	}
	inst->transition_fader.timer = t;
	inst->transition_fader.frames = inst->transition_fader.timer * Amphora_GetFPSV1() / 1000;
	inst->transition_fader.idx = 0;
	if (!((inst->transition_fader.steps = Amphora_HeapAlloc(inst->transition_fader.frames * sizeof(Uint8), MEM_MISC))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate memory for fade steps\n");
		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	for (i = 0; i < inst->transition_fader.frames; i++)
	{
		inst->transition_fader.steps[i] = i * 255 / (inst->transition_fader.frames - 1);
	}
	inst->tilemap_flags.transitioning = true;
	inst->tilemap_flags.persist_shown = true;
	(void)Amphora_RegisterEventV1("amph_internal_map_layer_fade", Amphora_MapLayerFadeEvent);

	return AMPHORA_STATUS_OK;
}

/*
 * Internal functions
 */

int
Amphora_InitMaps(void)
{
	int i;

	init.map_data = HT_NewTable();
	for (i = 0; i < init.map_count; i++)
	{
		HT_StoreRef(init.map_names[i], init.map_paths[i], init.map_data);
#ifdef DEBUG
		SDL_Log("Found map %s\n", init.map_names[i]);
#endif
	}

	init.obj_groups.i = HT_NewTable();
	if (!((init.obj_groups.rects = Amphora_HeapAlloc(HT_GetSize(init.obj_groups.i) * sizeof(SDL_FRect *), MEM_TILEMAPS))))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate object group rectangle list\n");
		return -1;
	}

	return 0;
}

void
Amphora_DestroyCurrentMap(void)
{
	int i;

	for (i = 0; i < inst->current_map.num_layers; i++)
	{
		if (inst->fade_layer == &inst->current_map.layers[i])
			inst->fade_layer = NULL;
		Amphora_HeapFree(inst->current_map.layer_names[i]);
		SDL_DestroyTexture(inst->current_map.layers[i].texture);
		inst->current_map.layers[i].node->garbage = true;
	}
	if (inst->current_map.num_layers > 0)
	{
		Amphora_HeapFree(inst->current_map.layers);
		Amphora_HeapFree(inst->current_map.layer_names);
		inst->current_map.layers = NULL;
		inst->current_map.layer_names = NULL;
	}
	inst->current_map.num_layers = 0;
}

void
Amphora_FreeObjectGroup(void)
{
	Uint32 i;

	for (i = 0; i < HT_GetCount(inst->obj_groups.i); i++)
	{
		Amphora_HeapFree(inst->obj_groups.rects[i]);
		inst->obj_groups.rects[i] = NULL;
	}

	HT_FreeTable(inst->obj_groups.i);
	inst->obj_groups.i = HT_NewTable();
	inst->obj_groups.rects = Amphora_HeapRealloc(inst->obj_groups.rects, HT_GetSize(inst->obj_groups.i) * sizeof(SDL_FRect *), MEM_TILEMAPS);
}

void
Amphora_FreeAllObjectGroups(void)
{
	Amphora_FreeObjectGroup();
	Amphora_HeapFree(inst->obj_groups.rects);
	HT_FreeTable(inst->obj_groups.i);
}

void
Amphora_CloseMapHashTables(void)
{
	HT_FreeTable(init.map_data);
}

SDL_FRect *
Amphora_GetRectsByGroup(const char *name, int *c)
{
	const int i = (int)HT_GetValue(name, inst->obj_groups.i);

	*c = (int)HT_GetStatus(name, inst->obj_groups.i);

	return inst->obj_groups.rects[i];
}

/*
 * Private functions
 */

static int
Amphora_ParseMapToTexture(const char *name)
{
	SDL_Renderer *renderer = Amphora_GetRenderer();
	cute_tiled_map_t *map = cute_tiled_load_map_from_file(HT_GetRef(name, char, inst->map_data), NULL);
	cute_tiled_layer_t *layer = map->layers;
	cute_tiled_tileset_t *tileset = map->tilesets;
	int tileset_img_w, tileset_img_h;
	SDL_Texture *tileset_img = Amphora_GetIMGTextureByName(tileset->name.ptr);
	int pixel_width = map->width * map->tilewidth;
	int pixel_height = map->height * map->tileheight;
	int map_base_w, map_base_h;
	int i;

	while (layer)
	{
		if (SDL_strcmp(layer->type.ptr, "tilelayer") == 0) inst->current_map.num_layers++;
		layer = layer->next;
	}
	if (!((inst->current_map.layers = Amphora_HeapAlloc(inst->current_map.num_layers * sizeof(struct amphora_tilemap_layer_t), MEM_TILEMAPS))))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not allocate map layers\n");
		return -1;
	}
	if (!((inst->current_map.layer_names = Amphora_HeapAlloc(inst->current_map.num_layers * sizeof(char *), MEM_TILEMAPS))))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not allocate map layers labels\n");
		return -1;
	}
	layer = map->layers;
	if (SDL_strcmp(map->orientation.ptr, "orthogonal") == 0)
	{
		inst->current_map.orientation = MAP_ORTHOGONAL;
		for (i = 0; i < inst->current_map.num_layers; i++)
		{
			inst->current_map.layers[i].texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
								  SDL_TEXTUREACCESS_TARGET, pixel_width,
								  pixel_height);
		}
	}
	else if (SDL_strcmp(map->orientation.ptr, "isometric") == 0)
	{
		inst->current_map.orientation = MAP_ISOMETRIC;
		for (i = 0; i < inst->current_map.num_layers; i++)
		{
			inst->current_map.layers[i].texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
								SDL_TEXTUREACCESS_TARGET,
								pixel_width + (map->tilewidth / 2), pixel_height);
		}
	}
	else
	{
		return -1;
	}
	(void)SDL_QueryTexture(tileset_img, NULL, NULL, &tileset_img_w, &tileset_img_h);
	i = 0;
	while (layer)
	{
		if (SDL_strcmp(layer->type.ptr, "tilelayer") == 0)
		{
			(void)Amphora_ParseTileLayer(map, layer, tileset_img_w, tileset_img, i++);
		}
		else if (SDL_strcmp(layer->type.ptr, "objectgroup") == 0)
		{
			(void)Amphora_ParseObjectGroup(layer);
		}
		layer = layer->next;
	}
	(void)SDL_SetRenderTarget(renderer, NULL);
	cute_tiled_free_map(map);
	(void)SDL_QueryTexture(inst->current_map.layers[0].texture, NULL, NULL, &map_base_w, &map_base_h);
	inst->map_rect.w = (float)map_base_w * inst->current_map.scale;
	inst->map_rect.h = (float)map_base_h * inst->current_map.scale;

	return 0;
}

static int
Amphora_ParseTileLayer(const cute_tiled_map_t *map, const cute_tiled_layer_t *layer, int tileset_img_w, SDL_Texture *tileset_img, int n)
{
	SDL_Renderer *renderer = Amphora_GetRenderer();
	SDL_Rect tile_s = { .w = map->tilewidth, .h = map->tileheight };
	SDL_Rect tile_d = { .w = map->tilewidth, .h = map->tileheight };
	int i, tile_idx, row;

	inst->current_map.layer_names[n] = Amphora_HeapStrdup(layer->name.ptr);
	if (inst->current_map.layer_names[n] == NULL)
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Could not allocate space for layer name!");
		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	(void)SDL_SetTextureBlendMode(inst->current_map.layers[n].texture, SDL_BLENDMODE_BLEND);
	(void)SDL_SetRenderTarget(renderer, inst->current_map.layers[n].texture);
	for (i = 0; i < layer->data_count; i++)
	{
		tile_idx = layer->data[i] - 1;
		row = ((i * map->tilewidth) / (map->width * map->tilewidth));
		tile_s.x = (tile_idx * map->tilesets->tilewidth) % tileset_img_w;
		tile_s.y = ((tile_idx * map->tilesets->tilewidth) / tileset_img_w) * map->tileheight;
		switch (inst->current_map.orientation)
		{
			case MAP_ORTHOGONAL:
				tile_d.x = (i * map->tilewidth) % (map->width * map->tilewidth);
				tile_d.y = row * map->tileheight;
				break;
			case MAP_ISOMETRIC:
				tile_d.x = (i * map->tilewidth) % (map->width * map->tilewidth) +
					   (((map->width * map->tilewidth) / 2) - ((i % map->width) *
										   (map->tilewidth /
										    2)) -
					    (row * (map->tilewidth / 2)));
				tile_d.y = row * map->tileheight +
					   ((i % map->width) * (map->tileheight / 2)) -
					   (row * (map->tileheight / 2));
				break;
		}
		(void)SDL_RenderCopy(renderer, tileset_img, &tile_s, &tile_d);
	}

	return AMPHORA_STATUS_OK;
}

static int
Amphora_ParseObjectGroup(const cute_tiled_layer_t *layer)
{
	int i;
	unsigned c = HT_GetCount(inst->obj_groups.i), s = HT_GetSize(inst->obj_groups.i);
	cute_tiled_object_t *object;

	(void)HT_SetValue(layer->name.ptr, c, inst->obj_groups.i);
	if (s != HT_GetSize(inst->obj_groups.i))
	{
		if (!((inst->obj_groups.rects = Amphora_HeapRealloc(inst->obj_groups.rects,
						      HT_GetSize(inst->obj_groups.i) * sizeof(SDL_FRect *), MEM_TILEMAPS))))
		{
			Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL,
				     "Failed to reallocate object group rectangle list!");
			return AMPHORA_STATUS_ALLOC_FAIL;
		}
	}
	object = layer->objects;
	i = 0;
	while (object)
	{
		i++;
		object = object->next;
	}
	object = layer->objects;
	if (!((inst->obj_groups.rects[c] = Amphora_HeapAlloc(i * sizeof(SDL_FRect), MEM_TILEMAPS))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate object group rectangles!");
		return AMPHORA_STATUS_ALLOC_FAIL;
	}
	(void)HT_SetStatus(layer->name.ptr, i, inst->obj_groups.i);
	i = 0;
	while (object)
	{
		inst->obj_groups.rects[c][i].x = object->x * inst->current_map.scale;
		inst->obj_groups.rects[c][i].y = object->y * inst->current_map.scale;
		inst->obj_groups.rects[c][i].w = object->width * inst->current_map.scale;
		inst->obj_groups.rects[c][i].h = object->height * inst->current_map.scale;
		i++;
		object = object->next;
	}

	return 0;
}

static int
Amphora_GetMapLayerByName(const char *name)
{
	int i;

	for (i = 0; i < inst->current_map.num_layers; i++)
	{
		if (SDL_strcmp(name, inst->current_map.layer_names[i]) == 0) return i;
	}
	return -1;
}

static void
Amphora_MapLayerFadeEvent(void)
{
	if (inst->fade_layer == NULL) goto cleanup;

	(void)SDL_SetTextureAlphaMod(inst->fade_layer->texture, inst->transition_fader.steps[inst->transition_fader.idx++]);
	if (inst->transition_fader.idx != inst->transition_fader.frames) return;

	inst->fade_layer->node->display = inst->tilemap_flags.persist_shown;

	cleanup:
		inst->tilemap_flags.transitioning = false;
		Amphora_HeapFree(inst->transition_fader.steps);
		(void)Amphora_UnregisterEventV1("amph_internal_map_layer_fade");
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterMapData(const char **names, const char **paths, int count)
{
	init.map_names = names;
	init.map_paths = paths;
	init.map_count = count;
}
