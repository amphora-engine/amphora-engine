#ifndef HASH_TABLE_HT_HASH_H
#define HASH_TABLE_HT_HASH_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_KEY_LEN 32

#define HT_StoreRef(key, val, tbl) (HT_SetValue((key), (uintptr_t)(val), (tbl)))
#define HT_GetRef(key, type, tbl) ((type *)HT_GetValue((key), (tbl)))

typedef struct hash_table_t * HT_HashTable;

const char *HT_GetError(void);
HT_HashTable HT_NewTable(void);
intptr_t HT_GetValue(const char *key, HT_HashTable t);
unsigned HT_SetValue(const char *key, intptr_t val, HT_HashTable t);
/* TODO: rename these to Get/SetUserData() */
intptr_t HT_GetStatus(const char *key, HT_HashTable t);
unsigned HT_SetStatus(const char *key, intptr_t val, HT_HashTable t);
unsigned HT_GetCount(HT_HashTable t);
unsigned HT_GetSize(HT_HashTable t);
void HT_DeleteKey(const char *key, HT_HashTable t);
void HT_FreeTable(HT_HashTable tbl);

#endif /* HASH_TABLE_HT_HASH_H */
