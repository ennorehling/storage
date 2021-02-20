#include "textstore.h"
#include "storage.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define isxspace(c) (c==160 || isspace(c))

static int txt_w_brk(HSTORAGE store)
{
    if (putc('\n', (FILE*)store.data) == EOF) return EOF;
    return 0;
}

static int txt_w_int(HSTORAGE store, int arg)
{
    int err = fprintf((FILE *)store.data, "%d ", arg);
    if (err < 0) {
        return err;
    }
    return 0;
}

static int txt_r_int(HSTORAGE store, int * result)
{
    int n, err = fscanf((FILE *)store.data, "%d", result ? result : &n);
    return (err == 1) ? 0 : err;
}

static int txt_w_flt(HSTORAGE store, float arg)
{
    int err = fprintf((FILE*)store.data, "%f ", arg);
    if (err < 0) {
        return err;
    }
    return 0;
}

static int txt_r_flt(HSTORAGE store, float * result)
{
    float flt;
    int err = fscanf((FILE *)store.data, "%f", result ? result : &flt);
    return (err == 1) ? 0 : err;
}

static int txt_w_tok(HSTORAGE store, const char *tok)
{
    int err;
    assert(tok);
    assert(strlen(tok) < TOKEN_MAXSIZE);
    err = fputs(tok, (FILE *)store.data);
    if (err < 0) return err;
    err = fputc(' ', (FILE *)store.data);
    return (err == ' ') ? 0 : err;
}

#define STR(a) #a
#define STRINGIZE(a) STR(a)
static int txt_r_tok_buf(HSTORAGE store, char *result, size_t size)
{
    FILE * F = (FILE *)store.data;
    int err;
    if (result) {
        if (size >= TOKEN_MAXSIZE) {
            err = fscanf(F, "%" STRINGIZE(TOKEN_MAXSIZE) "s", result);
        }
        else {
            result[0] = '\0';
            err = fscanf(F, "%*s");
        }
    }
    else {
        err = fscanf(F, "%*s");
    }
    return err == EOF ? EOF : 0;
}

static int freadstr(FILE * F, char *start, size_t size)
{
    char *str = start;
    int quote = 0;
    for (;;) {
        int c = fgetc(F);

        if (isxspace(c)) {
            if (str == start) {
                continue;
            }
            if (!quote) {
                *str = 0;
                return 0;
            }
        }
        switch (c) {
        case EOF:
            return EOF;
        case '"':
            if (!quote && str != start) {
                assert
                (!"file contains a \" that isn't at the start of a string.\n");
            }
            if (quote) {
                *str = 0;
                return 0;
            }
            quote = 1;
            break;
        case '\\':
            c = fgetc(F);
            switch (c) {
            case EOF:
                return EOF;
            case 'n':
                if ((size_t)(str - start + 1) < size) {
                    *str++ = '\n';
                }
                break;
            default:
                if ((size_t)(str - start + 1) < size) {
                    *str++ = (char)c;
                }
            }
            break;
        default:
            if ((size_t)(str - start + 1) < size) {
                *str++ = (char)c;
            }
        }
    }
}

static int fwritestr(FILE * F, const char *str)
{
    if (fputc('\"', F) == EOF) return EOF;
    if (str) {
        while (*str) {
            int c = (int)(unsigned char)*str++;
            switch (c) {
            case '"':
            case '\\':
                if (fputc('\\', F) == EOF) return EOF;
                if (fputc(c, F) == EOF) return EOF;
                break;
            case '\n':
                if (fputc('\\', F) == EOF) return EOF;
                if (fputc('n', F) == EOF) return EOF;
                break;
            default:
                if (fputc(c, F) == EOF) return EOF;
            }
        }
    }
    if (fputc('\"', F) == EOF) return EOF;
    return 0;
}

static int txt_w_str(HSTORAGE store, const char *str)
{
    int err = fwritestr((FILE *)store.data, str);
    if (err) return err;
    if (' ' != fputc(' ', (FILE*)store.data)) return EOF;
    return 0;
}

static int txt_r_str_buf(HSTORAGE store, char *result, size_t size)
{
    return freadstr((FILE *)store.data, result, size);
}

const storage_i text_api = {
    txt_w_brk,
    txt_w_int, txt_r_int,
    txt_w_flt, txt_r_flt,
    txt_w_tok, txt_r_tok_buf,
    txt_w_str, txt_r_str_buf,
};

void txtstore_init(struct storage * store, FILE *F) {
    store->api = &text_api;
    store->handle.data = F;
}

void txtstore_done(struct storage * store) {
    assert(store->api == &text_api);
    fclose((FILE *)store->handle.data);
}
