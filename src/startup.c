#include <stdio.h>
#include <stdlib.h>

#include "amphora.h"
#include "amphora_api.h"

static AmphoraStartup ast;
static AmphoraAPI_V1 aapi_v1;

static void *
Amphora_GetAPI(int version)
{
	switch (version)
	{
		case 1: return &aapi_v1;
		default: return NULL;
	}
}

AmphoraStartup *
Amphora_Connect(void)
{
	size_t version_string_len = strlen(ENGINE_SUFFIX) + strlen(VERSION_STRING) + 2;

#define AMPHORA_VFUNCTION_V1(ret, name, sig_args, p_sig_args, call_args) aapi_v1.name = Amphora_##name
#define AMPHORA_FUNCTION_V1(ret, name, sig_args, call_args) aapi_v1.name = Amphora_##name
#define AMPHORA_ROUTINE_V1(name, sig_args, call_args) aapi_v1.name = Amphora_##name
	#include "amphora_api.def"
#undef AMPHORA_VFUNCTION_V1
#undef AMPHORA_FUNCTION_V1
#undef AMPHORA_ROUTINE_V1

	ast.engine_version = malloc(version_string_len);
	if (ast.engine_version == NULL)
		(void)fputs("Failed to allocate memory for engine version string\n", stderr);
	else
		(void)snprintf(ast.engine_version, version_string_len, "%s-%s", ENGINE_SUFFIX, VERSION_STRING);

	ast.api_version = AMPHORA_API_VERSION;
	ast.StartEngine = Amphora_StartEngine;
	ast.RegisterGameData = Amphora_RegisterGameData;
	ast.RegisterWindowTitle = Amphora_RegisterWindowTitle;
	ast.RegisterPrefs = Amphora_RegisterPrefs;
	ast.RegisterActionData = Amphora_RegisterActionData;
	ast.RegisterSceneData = Amphora_RegisterSceneData;
	ast.RegisterImageData = Amphora_RegisterImageData;
	ast.RegisterFontData = Amphora_RegisterFontData;
	ast.RegisterMapData = Amphora_RegisterMapData;
	ast.RegisterSFXData = Amphora_RegisterSFXData;
	ast.RegisterMusicData = Amphora_RegisterMusicData;
	ast.GetAPI = Amphora_GetAPI;

	return &ast;
}
