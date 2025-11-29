#include "internal/ht_hash.h"
#include "internal/memory.h"

#include <errno.h>
#include <stdarg.h>

#define OFFSET 0x811c9dc5
#define PRIME 0x01000193
#define INIT_TBL_SIZE 8
#define MSG_LEN 256

enum hash_status_e
{
	HT_DELETED = -1,
	HT_FREE,
	HT_USED
};

struct hash_entry_t
{
	intptr_t data;
	intptr_t user_data;
	unsigned hash;
	enum hash_status_e status;
	char key[MAX_KEY_LEN];
};

struct hash_table_t
{
	int size, count;
	struct hash_entry_t *table_entries;
};

static char msg[MSG_LEN];

void
HT_SetError(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	(void)vsnprintf(msg, MSG_LEN, fmt, args);
}

const char *
HT_GetError(void)
{
	return msg;
}

HT_HashTable
HT_NewTable(void)
{
	HT_HashTable tbl;

	tbl = Amphora_HeapAlloc(sizeof(struct hash_table_t), MEM_HASHTABLE);
	if (tbl == NULL)
	{
		HT_SetError("Could not allocate table: %s", strerror(errno));
		return NULL;
	}
	tbl->table_entries = Amphora_HeapCalloc(INIT_TBL_SIZE,sizeof(struct hash_entry_t), MEM_HASHTABLE);
	if (tbl->table_entries == NULL)
	{
		HT_SetError("Could not allocate table data: %s", strerror(errno));
		Amphora_HeapFree(tbl);
		return NULL;
	}
	tbl->count = 0;
	tbl->size = INIT_TBL_SIZE;

	return tbl;
}

struct hash_table_t *
HT_IncreaseSizeRehash(struct hash_table_t *tbl) {
	struct hash_entry_t *ntbl = NULL, *otbl;
	int i;

	ntbl = Amphora_HeapCalloc(tbl->size << 1, sizeof(struct hash_entry_t), MEM_HASHTABLE);
	if (ntbl == NULL)
	{
		HT_SetError("Failed to grow table: %s", strerror(errno));
		return NULL;
	}
	tbl->count = 0;
	otbl = tbl->table_entries, tbl->table_entries = ntbl;
	tbl->size <<= 1;
	for (i = 0; i < tbl->size >> 1; i++)
	{
		if (otbl[i].status > 0)
		{
			(void)HT_SetValue(otbl[i].key, otbl[i].data, tbl);
			(void)HT_SetStatus(otbl[i].key, otbl[i].status, tbl);
		}
	}
	Amphora_HeapFree(otbl);

	return tbl;
}

unsigned
HT_GetHash(const char *data)
{
	unsigned hash = OFFSET;
	char *d = (char *)data;

	if (!d) return 0;
	while (*d)
	{
		hash ^= *d++;
		hash *= PRIME;
	}
	return hash;
}

int
HT_ProbeForBucket(const struct hash_table_t *t, const char *key, unsigned hash, int i, int set)
{
	int len = t->size, end_idx = i ? i - 1 : len, p = -1;

	while (t->table_entries[i].status != HT_FREE && (t->table_entries[i].hash != hash || strcmp(t->table_entries[i].key, key) != 0))
	{
		if (set && t->table_entries[i].status == HT_DELETED && p == -1) p = i;
		if (i == end_idx) return -1;
		if (++i == len) i = 0;
	}
	if (!set) return t->table_entries[i].status == HT_FREE ? -1 : i;

	return (t->table_entries[i].status == HT_FREE && p > -1) ? p : i;
}

intptr_t
HT_GetValue(const char *key, HT_HashTable t)
{
	unsigned hash = HT_GetHash(key);
	int i = (int)(hash & (t->size - 1));

	if (!key) return 0;
	if (t->table_entries[i].hash == hash && strcmp(t->table_entries[i].key, key) == 0)
	{
		if (t->table_entries[i].status == HT_DELETED) return -1;

		return t->table_entries[i].data;
	}
	i = HT_ProbeForBucket(t, key, hash, i, 0);
	if (i == -1) return -1;

	if (t->table_entries[i].hash != hash)
	{
		HT_SetError("Key %s does not exist in table", key);
		return -1;
	}
	return t->table_entries[i].data;
}

unsigned
HT_SetValue(const char *key, intptr_t val, HT_HashTable t)
{
	unsigned hash = HT_GetHash(key);
	int i;

	if (!key) return 0;
	if (t->count >= (t->size * 7) / 10) (void)HT_IncreaseSizeRehash(t);
	i = (int)(hash & (t->size - 1));
	if (t->table_entries[i].hash && (t->table_entries[i].hash != hash || strcmp(t->table_entries[i].key, key) != 0))
		i = HT_ProbeForBucket(t, key, hash, i, 1);
	if (i == -1)
	{
		HT_SetError("Table full, cannot accept key %s", key);
		return 0;
	}
	if (t->table_entries[i].status != HT_USED) t->count++;
	t->table_entries[i].data = val, t->table_entries[i].hash = hash, t->table_entries[i].status = HT_USED;
	(void)strlcpy(t->table_entries[i].key, key, MAX_KEY_LEN - 1);
	return i;
}

intptr_t
HT_GetStatus(const char *key, HT_HashTable t)
{
	unsigned hash = HT_GetHash(key);
	int i = (int)(hash & (t->size - 1));

	if (!key) return 0;
	if (t->table_entries[i].hash == hash && strcmp(t->table_entries[i].key, key) == 0)
		return t->table_entries[i].user_data;
	i = HT_ProbeForBucket(t, key, hash, i, 0);
	if (i == -1 || t->table_entries[i].hash != hash)
	{
		HT_SetError("Key %s does not exist in table", key);
		return 0;
	}
	return t->table_entries[i].user_data;
}

unsigned
HT_SetStatus(const char *key, intptr_t val, HT_HashTable t)
{
	unsigned hash = HT_GetHash(key);
	int i = (int)(hash & (t->size - 1));

	if (!key || t->table_entries[i].status != HT_USED) return 0;
	if (t->table_entries[i].hash && (t->table_entries[i].hash != hash || strcmp(t->table_entries[i].key, key) != 0))
		i = HT_ProbeForBucket(t, key, hash, i, 1);

	t->table_entries[i].user_data = val;

	return i;
}

unsigned
HT_GetCount(HT_HashTable t)
{
	return t->count;
}

unsigned
HT_GetSize(HT_HashTable t)
{
	return t->size;
}

void
HT_DeleteKey(const char *key, HT_HashTable t)
{
	unsigned hash = HT_GetHash(key);
	int i = (int)(hash & (t->size - 1));

	if (!key) return;
	if (t->table_entries[i].hash != hash || strcmp(t->table_entries[i].key, key) != 0)
		i = HT_ProbeForBucket(t, key, hash, i, 0);
	if (i == -1 || t->table_entries[i].status != HT_USED || t->table_entries[i].hash != hash)
	{
		HT_SetError("Key %s does not exist in table", key);
		return;
	}
	t->table_entries[i].status = HT_DELETED;
	t->count--;
}

void
HT_FreeTable(HT_HashTable tbl)
{
	Amphora_HeapFree(tbl->table_entries);
	Amphora_HeapFree(tbl);
}
