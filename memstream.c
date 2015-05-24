#include "stream.h"
#include "memstream.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGELEN 4096

typedef struct page {
    struct page * next;
    void * ptr;
} page;

typedef struct memstream {
    struct page * pages;
    struct page * active;
    void * pos;
} memstream;

static void free_pages(page ** pages) {
    while (*pages) {
        page * pg = *pages;
        *pages = pg->next;
        free(pg->ptr);
        free(pg);
    }
}

static size_t ms_read(HSTREAM s, void* out, size_t outlen) {
    memstream * ms = (memstream *)s.data;
    char *dst = (char *)out;
    char *start = out;

    while (*ms->pos && outlen > 0) {
        page * pg = *ms->pos;
        size_t bytes = pg->len > outlen ? outlen : pg->len;

        memcpy(dst, pg->ptr, bytes);
        dst += bytes;
        outlen -= bytes;
        ms->pos = &pg->next;
    }
    return dst - start;
}

static size_t ms_write(HSTREAM s, const void* out, size_t len) {
    memstream * ms = (memstream *)s.data;
    page ** ptr = ms->pos;
    page * pg = (page *)malloc(sizeof(page));
    if (pg) {
        pg->next = 0;
        pg->len = len;
        pg->ptr = (char *)malloc(len+1);
        memcpy(pg->ptr, out, len);
        pg->ptr[len]='\0';

        free_page(ptr);
        *ptr = pg;
        ms->pos = &pg->next;
        return len;
    }
    errno = ENOMEM;
    return 0;
}

static int ms_readln(HSTREAM s, char* out, size_t outlen) {
    memstream * ms = (memstream *)s.data;
    page * pg = *ms->pos;
    size_t len;

    if (pg == 0) {
        return EOF;
    }
    len = (pg->len > outlen ? outlen : pg->len) - 1;
    memcpy(out, pg->ptr, len);
    out[len] = 0;
    ms->pos = &pg->next;
    return 0;
}

static int ms_writeln(HSTREAM s, const char * out) {
    memstream * ms = (memstream *)s.data;
    page ** ptr = ms->pos;
    page * pg = (page *)malloc(sizeof(page));
    if (pg) {
        size_t len = strlen(out);
        pg->next = 0;
        pg->ptr = (char *)malloc(len + 2);
        memcpy(pg->ptr, out, len);
        pg->ptr[len] = '\n';
        pg->ptr[len + 1] = '\0';
        pg->len = len + 1;
        free_page(ptr);
        *ptr = pg;
        ms->pos = &pg->next;
        return 0;
    }
    return ENOMEM;
}

static void ms_rewind(HSTREAM s) {
    memstream * ms = (memstream *)s.data;
    ms->pos = &ms->ptr;
}

static const stream_i api = {
    ms_writeln,
    ms_write,
    ms_readln,
    ms_read,
    ms_rewind
};

void mstream_init(struct stream * strm) {
    memstream * ms = (memstream *)malloc(sizeof(memstream));
    if (ms) {
        ms->ptr = 0;
        ms->pos = &ms->ptr;
        strm->api = &api;
        strm->handle.data = ms;
    }
}

void mstream_done(struct stream * strm)
{
    memstream * ms = (memstream *)strm->handle.data;
    free_page(&ms->ptr);
    free(ms);
}
