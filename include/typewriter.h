#ifndef TYPEWRITER_H
#define TYPEWRITER_H

#define MAX_CONCURRENT_TYPEWRITERS 4

typedef enum {
	TYPEWRITER_ERROR,
	TYPEWRITER_NOSTRING,
	TYPEWRITER_CREATED,
	TYPEWRITER_WAITING,
	TYPEWRITER_ATTRIB_UPDATE,
	TYPEWRITER_ADVANCE,
	TYPEWRITER_DONE
} TypewriterStatus;

#ifdef __cplusplus
extern "C" {
#endif
TypewriterStatus Amphora_TypeStringV1(AmphoraString *string, int ms, void (*callback)(int, char));
TypewriterStatus Amphora_SetStringTypeSpeedV1(AmphoraString *string, int ms);
#ifdef __cplusplus
}
#endif

#endif /* TYPEWRITER_H */
