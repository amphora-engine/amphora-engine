#ifndef AMPHORA_DB_INTERNAL_H
#define AMPHORA_DB_INTERNAL_H

#include "sqlite3.h"

sqlite3 *Amphora_GetDB(void);
int Amphora_InitDB(void);
void Amphora_CloseDB(void);

#endif /* AMPHORA_DB_INTERNAL_H */
