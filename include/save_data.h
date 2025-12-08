#ifndef AMPHORA_SAVE_DATA_H
#define AMPHORA_SAVE_DATA_H

#ifdef __cplusplus
extern "C" {
#endif
/* Save an integer with keys attribute */
int Amphora_SaveNumberV1(const char *attribute, double value);
/* Save a string with keys attribute */
int Amphora_SaveStringV1(const char *attribute, const char *value);
/* Get the integer stored with keys attribute, returns default_value if attribute doesn't exist */
double Amphora_LoadNumberV1(const char *attribute, double default_value);
/* Get the string stored with keys attribute and store it in out_string */
int Amphora_LoadStringV1(const char *attribute, char **out_string);
#ifdef __cplusplus
}
#endif

#endif /* AMPHORA_SAVE_DATA_H */
