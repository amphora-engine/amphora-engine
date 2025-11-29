#include "internal/render.h"
#include "internal/typewriter.h"
#include "internal/ttf.h"

/*
 * TODO: fix memory leak in typewriters when a typewriter gets interrupted before finishing
 */

static struct amphora_typewriter_t typewriters[MAX_CONCURRENT_TYPEWRITERS];
static unsigned int typewriters_count;

TypewriterStatus
Amphora_TypeString(AmphoraString *string, Uint32 ms, void (*callback)(int, char))
{
	int i;

	if (!string) return TYPEWRITER_NOSTRING;
	if (Amphora_GetStringLength(string) == Amphora_GetNumCharactersDisplayed(string))
		return TYPEWRITER_DONE;

	for (i = 0; i < MAX_CONCURRENT_TYPEWRITERS; i++)
	{
		if (typewriters[i].string == string) break;
	}
	if (i == MAX_CONCURRENT_TYPEWRITERS)
	{
		if (typewriters_count == MAX_CONCURRENT_TYPEWRITERS)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot create typewriter, concurrent limit exceeded!\n");

			return TYPEWRITER_ERROR;
		}
		i = 0;
		while (typewriters[i].used) i++;
		typewriters[i].string = string;
		typewriters[i].ticker = 0;
		typewriters[i].ms = ms;
		typewriters[i].last_update = SDL_GetTicks();
		typewriters[i].used = true;
		(void)Amphora_UpdateStringCharsDisplayed(string, 1);
		typewriters_count++;
		return TYPEWRITER_CREATED;
	}
	if (SDL_GetTicks() - typewriters[i].last_update <= typewriters[i].ms) return TYPEWRITER_WAITING;

	typewriters[i].last_update = SDL_GetTicks();
	if (callback) callback(typewriters[i].ticker, Amphora_GetStringCharAtIndex(string, typewriters[i].ticker));
	(void)Amphora_UpdateStringCharsDisplayed(string, ++typewriters[i].ticker);
	if (typewriters[i].ticker == (ssize_t)Amphora_GetStringLength(string))
	{
		typewriters[i].used = false;
		typewriters[i].string = NULL;
		typewriters_count--;

		return TYPEWRITER_DONE;
	}

	return TYPEWRITER_ADVANCE;
}

TypewriterStatus
Amphora_SetStringTypeSpeed(AmphoraString *string, Uint32 ms)
{
	int i;

	if (!string) return TYPEWRITER_NOSTRING;
	if (Amphora_GetStringLength(string) == Amphora_GetNumCharactersDisplayed(string)) return TYPEWRITER_DONE;

	for (i = 0; i < MAX_CONCURRENT_TYPEWRITERS; i++)
	{
		if (typewriters[i].string == string) break;
	}
	if (i == MAX_CONCURRENT_TYPEWRITERS && typewriters_count == MAX_CONCURRENT_TYPEWRITERS)
		return TYPEWRITER_ERROR;

	typewriters[i].ms = ms;

	return TYPEWRITER_ATTRIB_UPDATE;
}
