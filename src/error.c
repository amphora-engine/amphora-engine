#include "internal/error.h"

/* File-scoped variables */
static char err_buff[AMPHORA_MSG_BUFF_SIZE];
static AmphoraStatusCode err_code;
static void (*catastrophe_handler)(void) = NULL;

const char *
Amphora_GetErrorV1(void)
{
	return err_buff;
}

AmphoraStatusCode
Amphora_GetErrorCodeV1(void)
{
	return err_code;
}

void
Amphora_SetCatastropheHandlerV1(void (*func)(void))
{
	catastrophe_handler = func;
}

/*
 * Internal functions
 */

void
Amphora_SetError(AmphoraStatusCode status_code, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	(void)SDL_vsnprintf(err_buff, AMPHORA_MSG_BUFF_SIZE, fmt, args);
	err_code = status_code;
}

void
Amphora_HandleCatastrophicFailure(void)
{
	/*
	 * TODO: implement catastrophe handler
	 */

	if (catastrophe_handler) catastrophe_handler();
}
