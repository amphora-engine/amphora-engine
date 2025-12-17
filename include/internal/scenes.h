#ifndef AMPHORA_SCENES_INTERNAL_H
#define AMPHORA_SCENES_INTERNAL_H

#include "../scenes.h"

typedef struct input_state_t InputState;

typedef struct amphora_scene_t {
	void (*init_func)(void);
	void (*update_func)(void);
	void (*destroy_func)(void);
} AmphoraScene;

void Amphora_InitSceneManager(void);
void Amphora_DeInitSceneManager(void);
void Amphora_InitScene(void);
void Amphora_UpdateScene(void);
void Amphora_DestroyScene(void);
bool Amphora_IsSceneUpdateLocked(void);
void Amphora_LockSceneUpdate(void);
void Amphora_UnlockSceneUpdate(void);

#endif /* AMPHORA_SCENES_INTERNAL_H */
