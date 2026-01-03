#ifndef PARTICLES_INTERNAL_H
#define PARTICLES_INTERNAL_H

#include "SDL.h"

#include "../particles.h"
#include "internal/render.h"

#define PARTICLE_GROUP_SIZE 15

/* The Rationale Behind 15:
 *
 * Given a memory pool size of 65536 bytes, a particle struct sizeof  48 bytes, and an allocation header size of 8 bytes;
 * letting k = number of particle groups per block, n = particles per group, and m = dead space...
 *
 * If we assume a pool filled only with particles, we can model the dead space as follows:
 * m = 65536 % (48n + 8)
 *
 * An n of 15 is the largest n that minimizes the value of m without consuming an entire pool,
 * so it was chosen as our group size.
 */

struct emitter_t {
	enum amphora_object_type_e type;
	AmphoraFRect rectangle;
	SDL_Texture *texture;
	AmphoraParticle **particles;
	int buckets_count;
	int particles_count;
	SDL_Color initial_color;
	SDL_FPoint start_position;
	int spread_x, spread_y;
	void (*update)(AmphoraParticle *, const AmphoraFRect *);
	struct render_list_node_t *render_list_node;
};

void Amphora_UpdateAndRenderParticleEmitter(AmphoraEmitter *emitter);

#endif /* PARTICLES_INTERNAL_H */
