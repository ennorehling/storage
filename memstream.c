#include "stream.h"
#include "memstream.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGELEN 4096

typedef struct page {
    struct page * next;
    char * ptr;
} page;

typedef struct memstream {
    struct page * pages;
    struct page * active;
    char *pos, *tail;
} memstream;

static page *alloc_page(void) {
    page * pg = malloc(sizeof(page) + PAGELEN);
    if (pg) {
        pg->next = 0;
        pg->ptr = (char *)(pg + 1);
    }
    return pg;
}

static void free_page(page * pg) {
    free(pg);
}

static size_t ms_read(HSTREAM s, void* out, size_t outlen) {
    memstream * ms = (memstream *)s.data;
    char *dst = (char *)out;
    char *start = out;
    char *src = ms->pos;
    page * pg = ms->active;

    while (src && outlen > 0) {
        size_t pglen = (ms->tail > pg->ptr && ms->tail - pg->ptr < PAGELEN) ? (ms->tail - pg->ptr) : PAGELEN;
        size_t bytes = pg->ptr + pglen - src;
        if (bytes > outlen) bytes = outlen;

        memcpy(dst, src, bytes);
        src += bytes;
        dst += bytes;
        outlen -= bytes;
        assert(src <= pg->ptr + pglen);
        ms->pos = src;
        if (src == pg->ptr + pglen) {
            if (!pg->next) {
                break;
            }
            pg = pg->next;
            src = pg->ptr;
        }
    }
    ms->active = pg;
    return dst - start;
}

static size_t ms_write(HSTREAM s, const void* out, size_t len) {
    memstream * ms = (memstream *)s.data;
    page *pg;
    char *dst;
    const char *src = (const char *)out;
    size_t outlen = len;

    if (!ms->pages) {
        ms->pages = ms->active = alloc_page();
        ms->pos = ms->active->ptr;
    }
    pg = ms->active;
    dst = ms->pos;
    while (outlen) {
        size_t bytes = pg->ptr + PAGELEN - dst;
        if (bytes==0) {
            if (!pg->next) {
                pg->next = alloc_page();
            }
            pg = pg->next;
            src = pg->ptr;
        }
        if (bytes > outlen) bytes = outlen;
        memcpy(dst, src, bytes);
        outlen -= bytes;
        dst += bytes;
        src += bytes;
    }
    ms->tail = ms->pos = dst;
    return len-outlen;
}

static int ms_readln(HSTREAM s, char* out, size_t outlen) {
    /*
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
    */
    return 0;
}

static int ms_writeln(HSTREAM s, const char * out) {
    /*
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
    */
    return ENOMEM;
}

static void ms_rewind(HSTREAM s) {
    memstream * ms = (memstream *)s.data;
    ms->active = ms->pages;
    ms->pos = ms->active ? ms->active->ptr : 0;
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
        ms->pages = 0;
        ms->pos = 0;
        strm->api = &api;
        strm->handle.data = ms;
    }
}

void mstream_done(struct stream * strm)
{
    memstream * ms = (memstream *)strm->handle.data;
    while (ms->pages) {
        page *pg = ms->pages;
        ms->pages = pg->next;
        free_page(pg);
    }
    free(ms);
    strm->handle.data = 0;
}
