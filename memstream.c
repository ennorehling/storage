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
    size_t pagesize;
    struct page * pages;
    struct page * active;
    char *pos, *tail;
} memstream;

static page *alloc_page(size_t size) {
    page * pg = malloc(sizeof(page) + size);
    if (pg) {
        pg->next = 0;
        pg->ptr = (char *)(pg + 1);
    }
    return pg;
}

static void free_page(page * pg) {
    free(pg);
}

static size_t ms_write(HSTREAM s, const void* out, size_t len) {
    memstream * ms = (memstream *)s.data;
    page *pg;
    char *dst;
    const char *src = (const char *)out;
    size_t outlen = len;

    if (!ms->pages) {
        ms->pages = ms->active = alloc_page(ms->pagesize);
        ms->pos = ms->active->ptr;
    }
    pg = ms->active;
    dst = ms->pos;
    while (outlen) {
        size_t bytes = pg->ptr + ms->pagesize - dst;
        if (bytes == 0) {
            if (!pg->next) {
                pg->next = alloc_page(ms->pagesize);
            }
            ms->active = pg->next;
            pg = pg->next;
            dst = pg->ptr;
        } else {
            if (bytes > outlen) bytes = outlen;
            memcpy(dst, src, bytes);
            outlen -= bytes;
            dst += bytes;
            src += bytes;
        }
    }
    ms->tail = ms->pos = dst;
    return len - outlen;
}

static size_t ms_read(HSTREAM s, void* out, size_t outlen) {
    memstream * ms = (memstream *)s.data;
    char *dst = (char *)out;
    char *start = out;
    char *src = ms->pos;
    page * pg = ms->active;

    while (src && outlen > 0) {
        size_t pglen = (ms->tail > pg->ptr && ms->tail - pg->ptr < ms->pagesize) ? (ms->tail - pg->ptr) : ms->pagesize;
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
            ms->active = pg = pg->next;
            ms->pos = src = pg->ptr;
        }
    }
    ms->active = pg;
    return dst - start;
}

static int ms_writeln(HSTREAM s, const char * out) {
    size_t len = strlen(out);
    size_t total;
    total = ms_write(s, out, len);
    total += ms_write(s, "\n", 1);
    return total-len-1;
}

static int ms_readln(HSTREAM s, char* out, size_t outlen) {
    memstream * ms = (memstream *)s.data;
    page * pg = ms->active;
    char *end, *src = ms->pos;

    while (pg && outlen > 0) {
        size_t pglen = (ms->tail > pg->ptr && ms->tail - pg->ptr < ms->pagesize) ? (ms->tail - pg->ptr) : ms->pagesize;

        end = (char *)memchr(src, '\n', pglen - (src - pg->ptr));
        if (!end) {
            size_t copy = pglen - (src - pg->ptr);
            if (copy > outlen) copy = outlen;
            if (copy == 0 && pg->next == 0) return EOF;
            memcpy(out, src, copy);
            outlen -= copy;
            out += copy;
            ms->active = pg = pg->next;
            if (pg == 0) {
                out[0] = '\0';
                return 0;
            }
            src = ms->pos = pg->ptr;
        }
        else if (end >= src) {
            size_t copy = end - src;
            if (copy > outlen) copy = outlen;
            ms->pos = end + 1;
            memcpy(out, src, copy);
            out[copy] = '\0';
            return 0;
        }
    }
    return EOF;
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

void mstream_init_size(struct stream * strm, size_t pagesize) {
    memstream * ms = (memstream *)malloc(sizeof(memstream));
    if (ms) {
        ms->pagesize = pagesize>0?pagesize:PAGELEN;
        ms->pages = 0;
        ms->pos = 0;
        ms->tail = 0;
        strm->api = &api;
        strm->handle.data = ms;
    }
}

void mstream_init(struct stream * strm) {
    mstream_init_size(strm, PAGELEN);
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
