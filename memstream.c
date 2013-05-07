#include "stream.h"
#include "memstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct strlist {
    struct strlist * next;
    char * str;
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

static int ms_readln(HSTREAM s, char * out, size_t outlen) {
    memstream * ms = (memstream *)s->data;
    strlist * list = *ms->pos;

    if (list==0) {
        return EOF;
    }
    strncpy(out, list->str, outlen);
    out[outlen-1] = 0;
    ms->pos = &list->next;
    return 0;
}

static int ms_writeln(HSTREAM s, const char * out) {
    memstream * ms = (memstream *)s->data;
    strlist ** ptr = ms->pos;
    strlist * list = (strlist *)malloc(sizeof(strlist));

    list->next = 0;
    list->str = (char *)malloc(strlen(out)+1);
    strcpy(list->str, out);

    free_strlist(ptr);
    *ptr = list;
    ms->pos = &list->next;

    return 0;
}
        
static void ms_rewind(HSTREAM s) {
    memstream * ms = (memstream *)s->data;
    ms->pos = &ms->ptr;
}

static const stream_i api = {
    ms_writeln,
    ms_readln,
    ms_rewind
};

void mstream_init(struct stream * strm) {
    memstream * ms = (memstream *)malloc(sizeof(memstream));

    ms->ptr = 0;
    ms->pos = &ms->ptr;
    strm->api = &api;
    strm->handle->data = (HSTREAM)ms;
}

void mstream_done(struct stream * strm)
{
    memstream * ms = (memstream *)strm->handle->data;
    free_strlist(&ms->ptr);
    free(ms);
}
