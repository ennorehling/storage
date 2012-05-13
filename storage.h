#ifndef STORAGE_H
#define STORAGE_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef void* HSTORAGE;

  typedef struct storage {
    /* separator for readable files: */
    int (*w_brk) (HSTORAGE store);
    /* integer values: */
    int (*w_int) (HSTORAGE store, int arg);
    int (*r_int) (HSTORAGE store);
    /* float values: */
    int (*w_flt) (HSTORAGE store, float arg);
    float (*r_flt) (HSTORAGE store);
    /* tokens that contain no whitespace: */
    int (*w_tok) (HSTORAGE store, const char *tok);
    int (*r_tok) (HSTORAGE store, char *result, size_t size);
    /* strings that need to be quoted: */
    int (*w_str) (HSTORAGE store, const char *tok);
    int (*r_str) (HSTORAGE store, char *result, size_t size);
    /* binary data: */
    int (*w_bin) (HSTORAGE store, void *arg, size_t size);
    int (*r_bin) (HSTORAGE store, void *result, size_t size);

    HSTORAGE (*begin) (FILE * F, int mode);
    int (*end) (HSTORAGE store);
  } storage;

#define IO_READ 0x01
#define IO_WRITE 0x02

#ifdef __cplusplus
}
#endif
#endif
