#ifndef AMPHORA_TYPEWRITER_INTERNAL_H
#define AMPHORA_TYPEWRITER_INTERNAL_H

#include "../typewriter.h"

struct amphora_typewriter_t {
	AmphoraString *string;
	Uint32 ms, last_update;
	int ticker;
	bool used : 1;
};

#endif /* AMPHORA_TYPEWRITER_INTERNAL_H */
