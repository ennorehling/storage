#include "stream.h"
#include "memstream.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct strlist {
    struct strlist * next;
    char * str;
    size_t len;
} strlist;

typedef struct memstream {
    struct strlist * ptr;
    struct strlist ** pos;
} memstream;

static void free_strlist(strlist ** ptr) {
    while (*ptr) {
        strlist * list = *ptr;
        *ptr = list->next;
        free(list->str);
        free(list);
    }
}

static int ms_readln(HSTREAM s, char* out, size_t outlen) {
    memstream * ms = (memstream *)s.data;
    strlist * list = *ms->pos;
    size_t len;

    if (list == 0) {
        return EOF;
    }
    len = (list->len > outlen ? outlen : list->len)-1;
    memcpy(out, list->str, len);
    out[len] = 0;
    ms->pos = &list->next;
    return 0;
}

static size_t ms_read(HSTREAM s, void* out, size_t outlen) {
    memstream * ms = (memstream *)s.data;
    char *dst = (char *)out;
    char *start = out;

    while (*ms->pos && outlen > 0) {
        strlist * list = *ms->pos;
        size_t bytes = list->len > outlen ? outlen : list->len;

        memcpy(dst, list->str, bytes);
        dst[bytes]='\0';
        dst += bytes;
        outlen -= bytes;
        ms->pos = &list->next;
    }
    return dst - start;
}

static int ms_writeln(HSTREAM s, const char * out) {
    memstream * ms = (memstream *)s.data;
    strlist ** ptr = ms->pos;
    strlist * list = (strlist *)malloc(sizeof(strlist));
    if (list) {
        size_t len = strlen(out);
        list->next = 0;
        list->str = (char *)malloc(len + 2);
        memcpy(list->str, out, len);
        list->str[len] = '\n';
        list->str[len+1] = '\0';
        list->len = len+1;
        free_strlist(ptr);
        *ptr = list;
        ms->pos = &list->next;
        return 0;
    }
    return ENOMEM;
}

static int ms_write(HSTREAM s, const void* out, size_t len) {
    memstream * ms = (memstream *)s.data;
    strlist ** ptr = ms->pos;
    strlist * list = (strlist *)malloc(sizeof(strlist));
    if (list) {
        list->next = 0;
        list->len = len;
        list->str = (char *)malloc(len+1);
        memcpy(list->str, out, len);
        list->str[len]='\0';

        free_strlist(ptr);
        *ptr = list;
        ms->pos = &list->next;
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
    free_strlist(&ms->ptr);
    free(ms);
}
