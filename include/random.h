#ifndef AMPHORA_RANDOM_H
#define AMPHORA_RANDOM_H

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Get a 16-bit random number less than n */
int Amphora_GetRandom(int n);
/* Get a random float 0.0 to 1.0 */
float Amphora_GetRandomF(void);
#ifdef __cplusplus
}
#endif

#endif /* AMPHORA_RANDOM_H */
