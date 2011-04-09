#ifndef STORAGE_H
#define STORAGE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct storage {
    /* separator for readable files: */
    int (*w_brk) (struct storage *);
    /* integer values: */
    int (*w_int) (struct storage *, int arg);
    int (*r_int) (struct storage *);
    /* float values: */
    int (*w_flt) (struct storage *, float arg);
    float (*r_flt) (struct storage *);
    /* tokens that contain no whitespace: */
    int (*w_tok) (struct storage *, const char *tok);
    int (*r_tok) (struct storage *, char *result, size_t size);
    /* strings that need to be quoted: */
    int (*w_str) (struct storage *, const char *tok);
    int (*r_str) (struct storage *, char *result, size_t size);
    /* binary data: */
    int (*w_bin) (struct storage *, void *arg, size_t size);
    int (*r_bin) (struct storage *, void *result, size_t size);

    int (*open) (struct storage *, const char *filename, int mode);
    int (*close) (struct storage *);

    void *userdata;
  } storage;

#define IO_READ 0x01
#define IO_WRITE 0x02

#ifdef __cplusplus
}
#endif
#endif
