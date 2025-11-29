#ifndef AMPHORA_TILEMAP_INTERNAL_H
#define AMPHORA_TILEMAP_INTERNAL_H

#include "../tilemap.h"

enum map_orientation_e {
	MAP_ORTHOGONAL,
	MAP_ISOMETRIC
};

struct amphora_tilemap_layer_t {
	SDL_Texture *texture;
	struct render_list_node_t *node;
};

struct amphora_tilemap_t {
	struct amphora_tilemap_layer_t *layers;
	char **layer_names;
	int num_layers;
	float scale;
	enum map_orientation_e orientation;
};

struct amphora_object_groups_t {
	HT_HashTable i;
	SDL_FRect **rects;
};

int Amphora_InitMaps(void);
void Amphora_DestroyCurrentMap(void);
void Amphora_FreeObjectGroup(void);
void Amphora_FreeAllObjectGroups(void);
void Amphora_CloseMapHashTables(void);
SDL_FRect *Amphora_GetRectsByGroup(const char *name, int *c);

#endif /* AMPHORA_TILEMAP_INTERNAL_H */
