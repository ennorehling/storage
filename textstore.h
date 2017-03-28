#ifndef H_STORAGE_TEXTSTORE
#define H_STORAGE_TEXTSTORE

#include <stdio.h>

/* HACK: maximum length of a token, including zero terminator
 * the code will assert when trying to store a token with more bytes,
 * and it will fail to read tokens when given a smaller buffer.
 */
#define TOKEN_MAXSIZE 32

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
