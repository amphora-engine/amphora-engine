#include "internal/context.h"
#include "internal/error.h"
#include "internal/memory.h"
#include "internal/particles.h"
#include "internal/random.h"
#include "internal/render.h"
#include "internal/system.h"

/* Prototypes for private functions */
ssize_t Amphora_CalculateOptimalGroupSize(void);
SDL_FPoint Amphora_CalculateParticleStartPosition(float start_x, float start_y, int spread_x, int spread_y);

static struct particles_ctx init;
static struct particles_ctx *inst = &init;

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
			void (*update_fn)(AmphoraParticle *, const AmphoraFRect *))
{
	AmphoraEmitter *emitter = NULL;
	struct render_list_node_t *render_list_node = NULL;
	SDL_FPoint position;
	SDL_Color initial_color = { color.r, color.g, color.b, color.a };
	ssize_t buckets_count;
	int i, j;

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
	buckets_count = count / inst->particle_group_size;
	if (buckets_count * inst->particle_group_size < count) buckets_count++;
	if (!((emitter->particles = Amphora_HeapAlloc(buckets_count * sizeof(AmphoraParticle *), MEM_EMITTER))))
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate particles");
		goto fail_buckets;
	}
	for (i = 0; i < buckets_count; i++)
	{
		emitter->particles[i] = Amphora_HeapAlloc(inst->particle_group_size * sizeof(AmphoraParticle), MEM_PARTICLE);
		if (emitter->particles[i] == NULL)
		{
			Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to allocate particles");
			goto fail_particles;
		}
	}
	emitter->buckets_count = buckets_count;
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

	for (i = 0; i < buckets_count; i++)
	{
		for (j = 0; j < inst->particle_group_size; j++)
		{
			if (i * inst->particle_group_size + j > count) break;

			position = Amphora_CalculateParticleStartPosition(start_x, start_y, spread_x, spread_y);
			emitter->particles[i][j].x = position.x;
			emitter->particles[i][j].y = position.y;
			emitter->particles[i][j].w = p_w;
			emitter->particles[i][j].h = p_h;
			emitter->particles[i][j].color = initial_color;
			emitter->particles[i][j].data1 = 0;
			emitter->particles[i][j].data2 = 0;
			emitter->particles[i][j].data3 = 0;
			emitter->particles[i][j].data4 = 0;
			emitter->particles[i][j].hidden = false;
		}
	}

	return emitter;

	fail_particles:
		for (i = 0; i < buckets_count; i++) Amphora_HeapFree(emitter->particles[i]);
		Amphora_HeapFree(emitter->particles);
	fail_buckets:
		SDL_DestroyTexture(emitter->texture);
	fail_texture:
		render_list_node->garbage = true;
		Amphora_HeapFree(emitter);
		return NULL;
}

int
Amphora_DestroyEmitterV1(AmphoraEmitter *emitter)
{
	int i;

	if (!emitter) return AMPHORA_STATUS_FAIL_UNDEFINED;
	if (Amphora_IsEngineRunning() == false) return AMPHORA_STATUS_OK;

	SDL_DestroyTexture(emitter->texture);
	for (i = 0; i < emitter->buckets_count; i++)
		Amphora_HeapFree(emitter->particles[i]);
	Amphora_HeapFree(emitter->particles);
	emitter->render_list_node->garbage = true;
	Amphora_HeapFree(emitter);

	return AMPHORA_STATUS_OK;
}

/*
 * Internal functions
 */

void
Amphora_InitParticleSystem(void)
{
	init.particle_group_size = Amphora_CalculateOptimalGroupSize();
}

void
Amphora_UpdateAndRenderParticleEmitter(AmphoraEmitter *emitter)
{
	SDL_Renderer *renderer = Amphora_GetRenderer();
	SDL_Color color = { 0, 0, 0, 0 };
	SDL_FRect dst;
	SDL_FRect target;
	Camera camera = Amphora_GetCameraV1();
	int i, j;

	(void)SDL_SetRenderTarget(renderer, emitter->texture);
	(void)SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	(void)SDL_RenderClear(renderer);

	for (i = 0; i < emitter->buckets_count; i++)
	{
		for (j = 0; j < inst->particle_group_size; j++)
		{
			if (i * inst->particle_group_size + j > emitter->particles_count) break;

			if (emitter->update) emitter->update(&emitter->particles[i][j], &emitter->rectangle);
			if (emitter->particles[i][j].hidden) continue;
			if (SDL_memcmp(&color, &emitter->particles[i][j].color, sizeof(SDL_Color)) != 0)
			{
				(void)SDL_memcpy(&color, &emitter->particles[i][j].color, sizeof(SDL_Color));
				(void)SDL_SetRenderDrawColor(
					renderer,
					color.r,
					color.g,
					color.b,
					color.a
				);
			}
			dst = (SDL_FRect){
				emitter->particles[i][j].x,
				emitter->particles[i][j].y,
				emitter->particles[i][j].w,
				emitter->particles[i][j].h
			};
			if (!emitter->render_list_node->stationary)
			{
				dst.x -= camera.x;
				dst.y -= camera.y;
			}
			(void)SDL_RenderFillRectF(renderer, &dst);
		}
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

ssize_t
Amphora_CalculateOptimalGroupSize(void)
{
	ssize_t min = -1;
	size_t n = 2; /* Particles per group */
	size_t m; /* Dead space in a full pool */
	size_t s; /* Total size of each allocation of a group of n particles */

	while (true)
	{
		s = sizeof(AmphoraParticle) * n + sizeof(struct amphora_mem_allocation_header_t);
		if (s > (AMPHORA_HEAP_SIZE >> 1) - sizeof(struct amphora_mem_allocation_header_t) * 2) break;

		m = AMPHORA_HEAP_SIZE % s;

		if (min == -1 || AMPHORA_HEAP_SIZE % (sizeof(AmphoraParticle) * min + sizeof(struct amphora_mem_allocation_header_t)) >= m)
			min = (ssize_t)n;

		n++;
	}

#ifdef DEBUG
	SDL_Log("Optimal particle group size: %ld\n", min);
#endif

	return min;
}

SDL_FPoint
Amphora_CalculateParticleStartPosition(float start_x, float start_y, int spread_x, int spread_y)
{
	return (SDL_FPoint) {
		start_x + (float)(Amphora_GetRandomV1(spread_x) - spread_x / 2.0),
		start_y + (float)(Amphora_GetRandomV1(spread_y) - spread_y / 2.0),
	};
}
