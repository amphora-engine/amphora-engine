#include "internal/collision.h"
#include "internal/img.h"
#include "internal/tilemap.h"

bool
Amphora_CheckCollision(const void *obj_a, const void *obj_b)
{
	if (obj_a == NULL || obj_b == NULL)
		return false;

	return SDL_HasIntersectionF(&((const IAmphoraCollidable *)obj_a)->rectangle, &((const IAmphoraCollidable *)obj_b)->rectangle);
}

AmphoraCollision
Amphora_CheckObjectGroupCollision(const void *obj, const char *name)
{
	int i, j;
	int c;
	float o_sx, o_ex, o_sy, o_ey, r_sx, r_ex, r_sy, r_ey;
	float overlaps[4];
	AmphoraCollision dirs[] = {
		AMPHORA_COLLISION_LEFT,
		AMPHORA_COLLISION_RIGHT,
		AMPHORA_COLLISION_TOP,
		AMPHORA_COLLISION_BOTTOM
	};
	int min_overlap = 0;
	SDL_FRect *rects;
	const IAmphoraCollidable *generic_obj = obj;

	if (obj == NULL)
		return AMPHORA_COLLISION_NONE;
	if ((rects = Amphora_GetRectsByGroup(name, &c)) == NULL)
		return AMPHORA_COLLISION_NONE;

	for (i = 0; i < c; i++)
	{
		if (SDL_HasIntersectionF(&generic_obj->rectangle, &rects[i]))
		{
			o_sx = generic_obj->rectangle.x;
			o_ex = o_sx + generic_obj->rectangle.w;
			o_sy = generic_obj->rectangle.y;
			o_ey = o_sy + generic_obj->rectangle.h;
			r_sx = rects[i].x;
			r_ex = r_sx + rects[i].w;
			r_sy = rects[i].y;
			r_ey = r_sy + rects[i].h;

			overlaps[0] = SDL_fabsf(o_ex - r_sx);
			overlaps[1] = SDL_fabsf(o_sx - r_ex);
			overlaps[2] = SDL_fabsf(o_ey - r_sy);
			overlaps[3] = SDL_fabsf(o_sy - r_ey);

			for (j = 1; j < 4; j++)
			{
				if (overlaps[j] < overlaps[min_overlap])
					min_overlap = j;
			}

			return dirs[min_overlap];
		}
	}

	return AMPHORA_COLLISION_NONE;
}
