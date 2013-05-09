#ifndef H_STORAGE_TEXTSTORE
#define H_STORAGE_TEXTSTORE

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct storage;
void txtstore_init(struct storage * store, FILE * F);
void txtstore_done(struct storage * store);

#ifdef __cplusplus
}
#endif
#endif
