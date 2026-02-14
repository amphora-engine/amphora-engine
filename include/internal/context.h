#ifndef AMPHORA_CONTEXT_H
#define AMPHORA_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#include "sqlite3.h"

#include "internal/error.h"
#include "internal/ht_hash.h"
#include "internal/lib.h"
#include "internal/memory.h"
#include "internal/mixer.h"
#include "internal/render.h"
#include "internal/scenes.h"
#include "internal/tilemap.h"
#include "internal/typewriter.h"

struct amphora_ctx
{
	uint32_t frame_count;
	uint32_t framerate;
	struct
	{
		bool quit_requested : 1;
		bool engine_running : 1;
	} engine_flags;
};

struct db_ctx
{
	sqlite3 *game_db;
	const char *game_author;
	const char *game_title;
};

struct error_ctx
{
	char err_buff[AMPHORA_MSG_BUFF_SIZE];
	AmphoraStatusCode err_code;
	void (*catastrophe_handler)(void);
};

struct events_ctx
{
	char **ev_names;
	int ev_count;
	int ev_max;
	HT_HashTable ev_table;
};

struct img_ctx
{
	HT_HashTable images;
	HT_HashTable image_surfaces;
	HT_HashTable image_surfaces_orig;
	HT_HashTable open_images;
	const char **img_names;
	const char **img_paths;
	int img_count;
};

struct input_ctx
{
	uint32_t *key_actions;
	SDL_GameController *controller;
	const char **action_names;
	int action_count;
	SDL_Keycode *keys;
	SDL_GameControllerButton *controller_buttons;
	const char **controller_button_names;
	SDL_Keycode pressed_key;
	bool joystickl_active;
	bool joystickr_active;
	Vector2f joystickl_state;
	Vector2f joystickr_state;
};

struct memory_ctx
{
	AmphoraMemBlock *amphora_heap;
	struct
	{
		AmphoraMemBlock data;
		uint16_t idx;
	} amphora_frame_heap;
	struct amphora_mem_block_metadata_t *heap_metadata;
	uint8_t current_block_categories[MEM_COUNT];
	bool shm_linked;
	const char **category_names;
};

struct mixer_ctx
{
	HT_HashTable sfx;
	HT_HashTable music;
	HT_HashTable open_sfx;
	Mix_Music *current_music;
	const char **sfx_names;
	const char **sfx_paths;
	int sfx_count;
	const char **music_names;
	const char **music_paths;
	int music_count;
};

struct particles_ctx
{
	ssize_t particle_group_size;
};

struct prefs_ctx
{
	char uuid[33];
	const char *game_author;
	const char *game_title;
	int window_x;
	int window_y;
	unsigned int window_flags;
	int framerate;
};

struct random_ctx
{
	uint32_t rand_state;
};

struct render_ctx
{
	SDL_Renderer *renderer;
	SDL_Window *window;
	const char *window_title;
	Camera camera;
	enum camera_mode_e camera_mode;
	SDL_Color bg;
	Vector2 render_logical_size;
	struct render_list_node_t *render_list;
	struct render_list_node_t *render_list_head;
	AmphoraImage *camera_target;
	SDL_FRect camera_boundary;
	uint32_t render_list_node_count;
};

struct scenes_ctx
{
	HT_HashTable scenes;
	const AmphoraScene *scene_structs;
	const char **scene_names;
	int scenes_count;
	long current_scene_idx;
	int current_scene_name;
	AmphoraFader transition_fader;
	SDL_Color fade_color;
	SDL_Rect fade_rect;
	bool scene_update_lock;
	unsigned long generation;
};

struct session_data_ctx
{
	HT_HashTable sd;
};

struct tilemap_ctx
{
	const char **map_names;
	const char **map_paths;
	int map_count;
	HT_HashTable map_data;
	struct amphora_tilemap_t current_map;
	AmphoraFader transition_fader;
	struct
	{
		bool transitioning : 1;
		bool persist_shown : 1;
	} tilemap_flags;
	struct amphora_tilemap_layer_t *fade_layer;
	AmphoraFRect map_rect;
	struct amphora_object_groups_t obj_groups;
};

struct ttf_ctx
{
	HT_HashTable fonts;
	HT_HashTable open_fonts;
	const char **font_names;
	const char **font_paths;
	int font_count;
};

struct typewriter_ctx
{
	struct amphora_typewriter_t typewriters[MAX_CONCURRENT_TYPEWRITERS];
	unsigned int typewriters_count;
};

#endif /* AMPHORA_CONTEXT_H */
