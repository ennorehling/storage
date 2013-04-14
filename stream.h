#ifndef STORAGE_STREAM_H
#define STORAGE_STREAM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef void* HSTREAM;

    typedef struct stream {
        int gets(HSTREAM s, char * out, size_t outlen);
        size_t read(HSTREAM s, char * out, size_t outlen);

        int puts(HSTREAM s, const char * ptr);
        size_t write(HSTREAM s, const void * out, size_t size);
        
        int rewind(HSTREAM s);
    } stream;

#ifdef __cplusplus
}
#endif
   
#endif
