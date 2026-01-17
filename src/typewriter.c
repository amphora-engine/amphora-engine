#include "internal/context.h"
#include "internal/render.h"
#include "internal/typewriter.h"
#include "internal/ttf.h"

/*
 * TODO: fix memory leak in typewriters when a typewriter gets interrupted before finishing
 */

static struct typewriter_ctx init;
static struct typewriter_ctx *inst = &init;

TypewriterStatus
Amphora_TypeStringV1(AmphoraString *string, int ms, void (*callback)(int, char))
{
	int i;

	if (!string) return TYPEWRITER_NOSTRING;
	if (Amphora_GetStringLengthV1(string) == Amphora_GetNumCharactersDisplayedV1(string))
		return TYPEWRITER_DONE;
	if (ms <= 0) return TYPEWRITER_ERROR;

	for (i = 0; i < MAX_CONCURRENT_TYPEWRITERS; i++)
	{
		if (inst->typewriters[i].string == string) break;
	}
	if (i == MAX_CONCURRENT_TYPEWRITERS)
	{
		if (inst->typewriters_count == MAX_CONCURRENT_TYPEWRITERS)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot create typewriter, concurrent limit exceeded!\n");

			return TYPEWRITER_ERROR;
		}
		i = 0;
		while (inst->typewriters[i].used) i++;
		inst->typewriters[i].string = string;
		inst->typewriters[i].ticker = 0;
		inst->typewriters[i].ms = ms;
		inst->typewriters[i].last_update = SDL_GetTicks();
		inst->typewriters[i].used = true;
		Amphora_UpdateStringCharsDisplayedV1(string, 1);
		inst->typewriters_count++;
		return TYPEWRITER_CREATED;
	}
	if (SDL_GetTicks() - inst->typewriters[i].last_update <= inst->typewriters[i].ms)
		return TYPEWRITER_WAITING;

	inst->typewriters[i].last_update = SDL_GetTicks();
	if (callback) callback(inst->typewriters[i].ticker, Amphora_GetStringCharAtIndexV1(string, inst->typewriters[i].ticker));
	Amphora_UpdateStringCharsDisplayedV1(string, ++inst->typewriters[i].ticker);
	if (inst->typewriters[i].ticker == (ssize_t)Amphora_GetStringLengthV1(string))
	{
		inst->typewriters[i].used = false;
		inst->typewriters[i].string = NULL;
		inst->typewriters_count--;

		return TYPEWRITER_DONE;
	}

	return TYPEWRITER_ADVANCE;
}

TypewriterStatus
Amphora_SetStringTypeSpeedV1(AmphoraString *string, int ms)
{
	int i;

	if (!string) return TYPEWRITER_NOSTRING;
	if (Amphora_GetStringLengthV1(string) == Amphora_GetNumCharactersDisplayedV1(string)) return TYPEWRITER_DONE;
	if (ms <= 0) return TYPEWRITER_ERROR;

	for (i = 0; i < MAX_CONCURRENT_TYPEWRITERS; i++)
	{
		if (inst->typewriters[i].string == string) break;
	}
	if (i == MAX_CONCURRENT_TYPEWRITERS && inst->typewriters_count == MAX_CONCURRENT_TYPEWRITERS)
		return TYPEWRITER_ERROR;

	inst->typewriters[i].ms = ms;

	return TYPEWRITER_ATTRIB_UPDATE;
}
