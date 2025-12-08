#ifndef DISABLE_TILEMAP
#ifndef TILEMAP_H
#define TILEMAP_H

#ifdef __cplusplus
extern "C" {
#endif
void Amphora_SetMapV1(const char *name, float scale);
const AmphoraFRect *Amphora_GetMapRectangleV1(void);
int Amphora_HideMapLayerV1(const char *name, int t);
int Amphora_ShowMapLayerV1(const char *name, int t);
#ifdef __cplusplus
}
#endif

#endif /* TILEMAP_H */
#endif
