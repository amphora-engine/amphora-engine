#ifndef AMPHORA_SYSTEM_H
#define AMPHORA_SYSTEM_H

#include <stdbool.h>

bool Amphora_IsEngineRunning(void);
const unsigned int *Amphora_GetFrameAddress(void);

#endif /* AMPHORA_SYSTEM_H */
