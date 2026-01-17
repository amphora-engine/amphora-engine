#include "internal/context.h"
#include "internal/error.h"

static struct error_ctx init;
static struct error_ctx *inst = &init;

const char *
Amphora_GetErrorV1(void)
{
	return inst->err_buff;
}

AmphoraStatusCode
Amphora_GetErrorCodeV1(void)
{
	return inst->err_code;
}

void
Amphora_SetCatastropheHandlerV1(void (*func)(void))
{
	inst->catastrophe_handler = func;
}

/*
 * Internal functions
 */

void
Amphora_SetError(AmphoraStatusCode status_code, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	(void)SDL_vsnprintf(inst->err_buff, AMPHORA_MSG_BUFF_SIZE, fmt, args);
	inst->err_code = status_code;
}

void
Amphora_HandleCatastrophicFailure(void)
{
	/*
	 * TODO: implement catastrophe handler
	 */

	if (inst->catastrophe_handler) inst->catastrophe_handler();
}
