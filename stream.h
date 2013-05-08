#ifndef STORAGE_STREAM_H
#define STORAGE_STREAM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct HSTREAM {
        void* data;
    } HSTREAM;

    typedef struct stream_i {
        int (*writeln)(HSTREAM s, const char * out);
        int (*readln)(HSTREAM s, char * out, size_t outlen);
        void (*rewind)(HSTREAM s);
    } stream_i;

    typedef struct stream {
        const struct stream_i * api;
        HSTREAM handle;
    } stream;

#ifdef __cplusplus
}
#endif
   
#endif
