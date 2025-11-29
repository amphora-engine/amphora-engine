#include "internal/lib.h"
#include "internal/memory.h"

/*
 * Internal functions
 */

char *
Amphora_ConcatString(char **s1, const char *s2)
{
	size_t n = SDL_strlen(*s1) + SDL_strlen(s2) + 1;

	*s1 = Amphora_HeapRealloc(*s1, n, MEM_STRING);
	(void)SDL_strlcat(*s1, s2, n);

	return *s1;
}
