#include "internal/error.h"
#include "internal/ht_hash.h"
#include "internal/memory.h"
#include "internal/render.h"
#include "internal/system.h"
#include "internal/ttf.h"

#define AMPHORA_MAX_STR_LEN 4096

/* Prototypes for private functions */
static SDL_Texture *Amphora_RenderStringToTexture(AmphoraString *msg);

/* File-scoped variables */
static HT_HashTable fonts;
static HT_HashTable open_fonts;
static const char **font_names;
static const char **font_paths;
static int font_count;

AmphoraString *
Amphora_CreateStringV1(const char *font_name, const int pt, const float x, const float y, const int order, const AmphoraColor color, const bool stationary, const char *fmt, va_list args)
{
	struct render_list_node_t *render_list_node = Amphora_AddRenderListNode(order);
	struct amphora_message_t *msg;
	SDL_RWops *font_rw;
	SDL_Color initial_color = { color.r, color.g, color.b, color.a };
	char text[AMPHORA_MAX_STR_LEN];

	if (HT_GetValue(font_name, fonts) == -1)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to locate font %s\n", font_name);
		return NULL;
	}
	if (HT_GetValue(font_name, open_fonts) == -1)
	{
#ifdef DEBUG
		SDL_Log("Loading font: %s\n", font_name);
#endif
		font_rw = SDL_RWFromFile(HT_GetRef(font_name, char, fonts), "rb");
		HT_StoreRef(font_name, TTF_OpenFontRW(font_rw, 1, 16), open_fonts);
	}
	(void)SDL_vsnprintf(text, sizeof(text), fmt, args);

	/* TODO: cleanup on fail */
	if (!((msg = Amphora_HeapAlloc(sizeof(struct amphora_message_t), MEM_STRING))))
		return NULL;
	if (!((msg->text = Amphora_HeapAlloc(SDL_strlen(text) + 1, MEM_STRING))))
		return NULL;
	if (!((msg->n_buff = Amphora_HeapAlloc(SDL_strlen(text) + 1, MEM_STRING))))
		return NULL;

	msg->type = AMPH_OBJ_TXT;
	msg->font_ptr = HT_GetRef(font_name, TTF_Font, open_fonts);
	msg->pt = pt;
	msg->len = SDL_strlen(text);
	msg->n = 0;
	msg->color = initial_color;
	msg->rectangle.x = x;
	msg->rectangle.y = y;
	msg->render_list_node = render_list_node;
	render_list_node->type = AMPH_OBJ_TXT;
	render_list_node->data = msg;
	render_list_node->stationary = stationary;
	(void)SDL_strlcpy(msg->text, text, SDL_strlen(text) + 1);

	msg->texture = Amphora_RenderStringToTexture(msg);

	return msg;
}

void
Amphora_ShowStringV1(AmphoraString *msg)
{
	if (msg == NULL) return;

	msg->render_list_node->display = true;
}

void
Amphora_HideStringV1(AmphoraString *msg)
{
	if (msg == NULL) return;

	msg->render_list_node->display = false;
}

size_t
Amphora_GetStringLengthV1(const AmphoraString *msg)
{
	return msg->len;
}

size_t
Amphora_GetNumCharactersDisplayedV1(const AmphoraString *msg)
{
	return msg->n;
}

const char *
Amphora_GetStringTextV1(AmphoraString *msg)
{
	return msg->text;
}

char
Amphora_GetStringCharAtIndexV1(const AmphoraString *msg, int idx)
{
	return msg->text[idx];
}

Vector2
Amphora_GetStringDimensionsV1(const AmphoraString *msg)
{
	return (Vector2){ (int)msg->rectangle.w, (int)msg->rectangle.h };
}

AmphoraString *
Amphora_UpdateStringTextV1(AmphoraString *msg, const char *fmt, va_list args)
{
	char text[4096];

	(void)SDL_vsnprintf(text, 4096, fmt, args);

	msg->len = SDL_strlen(text);
	Amphora_HeapFree(msg->text);
	msg->len = SDL_strlen(text);
	msg->text = Amphora_HeapStrdup(text);
	if (!((msg->n_buff = Amphora_HeapRealloc(msg->n_buff, SDL_strlen(text) + 1, MEM_STRING))))
	{
		return NULL;
	}
	if (!msg->text)
	{
		Amphora_SetError(AMPHORA_STATUS_ALLOC_FAIL, "Failed to set new string: %s", text);
		return NULL;
	}
	SDL_DestroyTexture(msg->texture);
	msg->texture = Amphora_RenderStringToTexture(msg);

	return msg;
}

AmphoraString *
Amphora_UpdateStringCharsDisplayedV1(AmphoraString *msg, size_t n)
{
	if (n > msg->len) n = 0;
	msg->n = n;
	SDL_DestroyTexture(msg->texture);
	msg->texture = Amphora_RenderStringToTexture(msg);

	return msg;
}

AmphoraString *
Amphora_UpdateStringPositionV1(AmphoraString *msg, float x, float y)
{
	msg->rectangle.x = x;
	msg->rectangle.y = y;

	return msg;
}

void
Amphora_FreeStringV1(AmphoraString *msg)
{
	if (!msg) return;
	if (Amphora_IsEngineRunningV1() == false) return;

	SDL_DestroyTexture(msg->texture);
	Amphora_HeapFree(msg->text);
	Amphora_HeapFree(msg->n_buff);
	msg->render_list_node->garbage = true;
	Amphora_HeapFree(msg);
}

void
Amphora_RenderStringV1(const AmphoraString *msg)
{
	SDL_FRect pos_adj;
	const Vector2f camera = Amphora_GetCameraV1();
	Vector2 logical_size = Amphora_GetRenderLogicalSizeV1();

	if (msg->render_list_node->stationary)
	{
		pos_adj = (SDL_FRect){
			.x = msg->rectangle.x > 0 ? msg->rectangle.x : (float) Amphora_GetResolutionV1().x + msg->rectangle.x - msg->rectangle.w,
			.y = msg->rectangle.y > 0 ? msg->rectangle.y : (float) Amphora_GetResolutionV1().y + msg->rectangle.y - msg->rectangle.h,
			.w = msg->rectangle.w,
			.h = msg->rectangle.h
		};
	}
	else
	{
		pos_adj = (SDL_FRect){
			.x = msg->rectangle.x - camera.x,
			.y = msg->rectangle.y - camera.y,
			.w = msg->rectangle.w,
			.h = msg->rectangle.h
		};
	}

	if (msg->render_list_node->stationary) Amphora_SetRenderLogicalSize(Amphora_GetResolutionV1());
	Amphora_RenderTexture(msg->texture, NULL, &pos_adj, 0, 0);
	if (msg->render_list_node->stationary) Amphora_SetRenderLogicalSize(logical_size);
}

/*
 * Internal functions
 */

int
Amphora_InitFonts(void)
{
	int i;

	fonts = HT_NewTable();
	open_fonts = HT_NewTable();
	for (i = 0; i < font_count; i++)
	{
		HT_StoreRef(font_names[i], font_paths[i], fonts);
#ifdef DEBUG
		SDL_Log("Found font %s\n", font_names[i]);
#endif
	}

	return 0;
}

void
Amphora_FreeAllFonts(void)
{
	int i;

	for (i = 0; i < font_count; i++)
	{
		if (HT_GetValue(font_names[i], open_fonts) != -1)
		{
#ifdef DEBUG
            SDL_Log("Unloading font: %s\n", font_names[i]);
#endif
			TTF_CloseFont(HT_GetRef(font_names[i], TTF_Font, open_fonts));
			(void)HT_SetValue(font_names[i], 0, open_fonts);
			HT_DeleteKey(font_names[i], open_fonts);
		}
	}

}

void
Amphora_CloseFonts(void)
{
	Amphora_FreeAllFonts();
	HT_FreeTable(fonts);
	HT_FreeTable(open_fonts);
}

/*
 * Private functions
 */

static SDL_Texture *
Amphora_RenderStringToTexture(AmphoraString *msg)
{
	int pt = msg->pt;
	int msg_rect_w, msg_rect_h;
	size_t n = msg->n;
	const char *text = msg->text;
	const SDL_Color text_color = msg->color;
	TTF_Font *font = msg->font_ptr;
	SDL_Surface *surface = NULL;
	SDL_Texture *texture = NULL;

	if (n) (void)SDL_strlcpy(msg->n_buff, text, n + 1);
	if (TTF_SetFontSize(font, pt) == -1)
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to set font size\n");

	surface = TTF_RenderUTF8_Blended(font, n ? msg->n_buff : text, text_color);
	texture = SDL_CreateTextureFromSurface(Amphora_GetRenderer(), surface);
	(void)TTF_SizeUTF8(font, n ? msg->n_buff : text, &msg_rect_w, &msg_rect_h);
	msg->rectangle.w = (float)msg_rect_w;
	msg->rectangle.h = (float)msg_rect_h;
	SDL_FreeSurface(surface);

	return texture;
}

/*
 * Dependency Injection functions
 */

void
Amphora_RegisterFontData(const char **names, const char **paths, int count)
{
	font_names = names;
	font_paths = paths;
	font_count = count;
}
