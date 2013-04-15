#ifndef STORAGE_MEMSTREAM_H
#define STORAGE_MEMSTREAM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stream;
void mstream_init(struct stream * strm);
void mstream_done(struct stream * strm);

#ifdef __cplusplus
}
#endif
   
#endif
