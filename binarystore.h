#ifndef H_STORAGE_BINSTORE
#define H_STORAGE_BINSTORE
#ifdef __cplusplus
extern "C" {
#endif

#include "storage.h"

struct storage;
void binstore_init(struct storage * store, FILE * F);
void binstore_done(struct storage * store);

#ifdef __cplusplus
}
#endif
#endif
