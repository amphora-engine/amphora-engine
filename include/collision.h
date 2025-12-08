#ifndef AMPHORA_COLLISION_H
#define AMPHORA_COLLISION_H

#include "SDL.h"

#include "render.h"
#include "util.h"

typedef struct amphora_collidable_t IAmphoraCollidable;

typedef enum {
	AMPHORA_COLLISION_NONE,
	AMPHORA_COLLISION_LEFT,
	AMPHORA_COLLISION_RIGHT,
	AMPHORA_COLLISION_TOP,
	AMPHORA_COLLISION_BOTTOM
} AmphoraCollision;


#ifdef __cplusplus
extern "C" {
#endif
/* Returns true if two objects have collided */
bool Amphora_CheckCollisionV1(const void *obj_a, const void *obj_b);
/* Returns the direction of collision if the object has collided with any rectangle in the object group */
AmphoraCollision Amphora_CheckObjectGroupCollisionV1(const void *obj, const char *name);
#ifdef __cplusplus
}
#endif

#endif /* AMPHORA_COLLISION_H */
