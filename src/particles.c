#include "internal/error.h"
#include "internal/memory.h"
#include "internal/particles.h"
#include "internal/random.h"
#include "internal/render.h"
#include "internal/system.h"

/*
 * Prototypes for private functions
 */

SDL_FPoint Amphora_CalculateParticleStartPosition(float start_x, float start_y, int spread_x, int spread_y);

AmphoraEmitter *
Amphora_CreateEmitterV1(float x,
			float y,
			float w,
			float h,
			float start_x,
			float start_y,
			int spread_x,
			int spread_y,
			int count,
			float p_w,
			float p_h,
			AmphoraColor color,
			bool stationary,
			int order,
			void (*update_fn)(int, int, AmphoraParticle *, AmphoraParticleExt *, const AmphoraFRect *))
{
	AmphoraEmitter *emitter = NULL;
	struct render_list_node_t *render_list_node = NULL;
	SDL_FPoint position;
	SDL_Color initial_color = { color.r, color.g, color.b, color.a };
	int i;

	if ((emitter = Amphora_HeapAlloc(sizeof(AmphoraEmitter), MEM_EMITTER)) == NULL)
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to initialize emitter");

		return NULL;
	}
	render_list_node = Amphora_AddRenderListNode(order);

	emitter->type = AMPH_OBJ_EMITTER;
	if (!((emitter->texture = SDL_CreateTexture(Amphora_GetRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, (int)w, (int)h))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to create emitter texture");
		goto fail_texture;
	}
	emitter->rectangle = (AmphoraFRect) { x, y, w, h };
	if (!((emitter->particles = Amphora_HeapAlloc(count * sizeof(AmphoraParticle), MEM_EMITTER))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate particles");
		goto fail_particles;
	}
	if (!((emitter->particle_data = Amphora_HeapAlloc(count * sizeof(AmphoraParticleExt), MEM_EMITTER))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate particle data");
		goto fail_data;
	}
	emitter->particles_count = count;
	emitter->initial_color = initial_color;
	emitter->start_position = (SDL_FPoint) { start_x, start_y };
	emitter->spread_x = spread_x;
	emitter->spread_y = spread_y;
	emitter->update = update_fn;
	emitter->render_list_node = render_list_node;
	render_list_node->type = AMPH_OBJ_EMITTER;
	render_list_node->data = emitter;
	render_list_node->stationary = stationary;

	(void)SDL_SetTextureBlendMode(emitter->texture, SDL_BLENDMODE_BLEND);

	for (i = 0; i < count; i++)
	{
		position = Amphora_CalculateParticleStartPosition(start_x, start_y, spread_x, spread_y);
		emitter->particles[i].x = position.x;
		emitter->particles[i].y = position.y;
		emitter->particles[i].w = p_w;
		emitter->particles[i].h = p_h;
		emitter->particles[i].color = initial_color;
		emitter->particle_data[i].data1 = 0;
		emitter->particle_data[i].data2 = 0;
		emitter->particle_data[i].data3 = 0;
		emitter->particle_data[i].data4 = 0;
		emitter->particle_data[i].hidden = false;
	}

	return emitter;

	fail_data:
		Amphora_HeapFree(emitter->particles);
	fail_particles:
		SDL_DestroyTexture(emitter->texture);
	fail_texture:
		render_list_node->garbage = true;
		Amphora_HeapFree(emitter);
		return NULL;
}

int
Amphora_DestroyEmitterV1(AmphoraEmitter *emitter)
{
	if (!emitter) return AMPHORA_STATUS_FAIL_UNDEFINED;
	if (Amphora_IsEngineRunningV1() == false) return AMPHORA_STATUS_OK;

	SDL_DestroyTexture(emitter->texture);
	Amphora_HeapFree(emitter->particles);
	Amphora_HeapFree(emitter->particle_data);
	emitter->render_list_node->garbage = true;
	Amphora_HeapFree(emitter);

	return AMPHORA_STATUS_OK;
}

/*
 * Internal functions
 */

void
Amphora_UpdateAndRenderParticleEmitter(AmphoraEmitter *emitter)
{
	SDL_Renderer *renderer = Amphora_GetRenderer();
	SDL_Color color = { 0, 0, 0, 0 };
	SDL_FRect dst;
	SDL_FRect target;
	Camera camera = Amphora_GetCameraV1();
	int i;

	(void)SDL_SetRenderTarget(renderer, emitter->texture);
	(void)SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	(void)SDL_RenderClear(renderer);

	for (i = 0; i < emitter->particles_count; i++)
	{
		if (emitter->update) emitter->update(i, emitter->particles_count, emitter->particles, emitter->particle_data, &emitter->rectangle);
		if (emitter->particle_data[i].hidden) continue;
		if (SDL_memcmp(&color, &emitter->particles[i].color, sizeof(SDL_Color)) != 0)
		{
			(void)SDL_memcpy(&color, &emitter->particles[i].color, sizeof(SDL_Color));
			(void)SDL_SetRenderDrawColor(
				renderer,
				color.r,
				color.g,
				color.b,
				color.a
			);
		}
		dst = (SDL_FRect){
			emitter->particles[i].x,
			emitter->particles[i].y,
			emitter->particles[i].w,
			emitter->particles[i].h
		};
		if (!emitter->render_list_node->stationary)
		{
			dst.x -= camera.x;
			dst.y -= camera.y;
		}
		(void)SDL_RenderFillRectF(renderer, &dst);
	}
	(void)SDL_SetRenderTarget(renderer, NULL);
	target.x = emitter->rectangle.x;
	target.y = emitter->rectangle.y;
	target.w = emitter->rectangle.w;
	target.h = emitter->rectangle.h;
	Amphora_RenderTexture(emitter->texture, NULL, &target, 0, SDL_FLIP_NONE);
}

/*
 * Private functions
 */

SDL_FPoint
Amphora_CalculateParticleStartPosition(float start_x, float start_y, int spread_x, int spread_y)
{
	return (SDL_FPoint) {
		start_x + (float)(Amphora_GetRandomV1(spread_x) - spread_x / 2.0),
		start_y + (float)(Amphora_GetRandomV1(spread_y) - spread_y / 2.0),
	};
}
