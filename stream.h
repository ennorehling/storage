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
        int(*writeln)(HSTREAM s, const char *out);
        int(*write)(HSTREAM s, const void *out, size_t outlen);
        int(*readln)(HSTREAM s, char *out, size_t outlen);
        int(*read)(HSTREAM s, void *out, size_t outlen);
        void(*rewind)(HSTREAM s);
    } stream_i;

    typedef struct stream {
        const struct stream_i * api;
        HSTREAM handle;
    } stream;

#define sputs(s, out) (out)->api->writeln((out)->handle, (s))
#define swrite(s, size, count, out) (out)->api->write((out)->handle, (s), (size)*(count))

#ifdef __cplusplus
}
#endif

#endif
