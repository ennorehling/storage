#ifndef H_STORAGE_INTERFACE
#define H_STORAGE_INTERFACE

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct HSTORAGE {
    void* data;
  } HSTORAGE;

  typedef struct storage_i {
    /* separator for readable files: */
    int (*w_brk) (HSTORAGE store);
    /* integer values: */
    int (*w_int) (HSTORAGE store, int arg);
    int (*r_int) (HSTORAGE store, int *result);
    /* float values: */
    int (*w_flt) (HSTORAGE store, float arg);
    int (*r_flt) (HSTORAGE store, float * result);
    /* tokens that contain no whitespace: */
    int (*w_tok) (HSTORAGE store, const char *tok);
    int (*r_tok) (HSTORAGE store, char *result, size_t size);
    /* strings that need to be quoted: */
    int (*w_str) (HSTORAGE store, const char *tok);
    int (*r_str) (HSTORAGE store, char *result, size_t size);
    /* binary data: */
    int (*w_bin) (HSTORAGE store, const void *arg, size_t size);
    int (*r_bin) (HSTORAGE store, void *result, size_t size);
  } storage_i;
  
  typedef struct storage {
    const struct storage_i * api;
    HSTORAGE handle;
  } storage;


  /* convenience macros */
#define WRITE_SECTION(store) (store)->api->w_brk((store)->handle)
#define READ_STR(store, charptr, size) (store)->api->r_str((store)->handle, charptr, size)
#define WRITE_STR(store, str) (store)->api->w_str((store)->handle, str)
#define READ_TOK(store, charptr, size) (store)->api->r_str((store)->handle, charptr, size)
#define WRITE_TOK(store, str) (store)->api->w_tok((store)->handle, str)
#define READ_INT(store, intptr) (store)->api->r_int((store)->handle, intptr)
#define WRITE_INT(store, num) (store)->api->w_int((store)->handle, num)
#define READ_FLT(store, intptr) (store)->api->r_flt((store)->handle, intptr)
#define WRITE_FLT(store, num) (store)->api->w_flt((store)->handle, num)

#ifdef __cplusplus
}
#endif
#endif
