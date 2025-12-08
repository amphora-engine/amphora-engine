#ifndef PARTICLES_H
#define PARTICLES_H

#include "render.h"

typedef struct {
	float x, y, w, h, vx, vy;
	SDL_Color color;
} AmphoraParticle;

typedef struct {
	float data1, data2;
	int data3, data4;
	bool hidden;
} AmphoraParticleExt;

#ifdef __cplusplus
extern "C" {
#endif
/* Create a new particle emitter */
AmphoraEmitter *Amphora_CreateEmitterV1(float x, float y, float w, float h, float start_x, float start_y, int spread_x,
	int spread_y, int count, float p_w, float p_h, AmphoraColor color, bool stationary, int order,
	void (*update_fn)(int, int, AmphoraParticle *, AmphoraParticleExt *, const AmphoraFRect *));
/* Destroy a particle emitter */
int Amphora_DestroyEmitterV1(AmphoraEmitter *emitter);
#ifdef __cplusplus
}
#endif
#endif /* PARTICLES_H */
