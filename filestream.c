#include "stream.h"
#include "filestream.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static int fs_readln(HSTREAM s, char * out, size_t outlen) {
    char * eol;
    FILE * F = (FILE *)s.data;

    assert(outlen <= INT_MAX);
    if (!out) {
        return fseek(F, (long)outlen, SEEK_CUR);
    }
    if (fgets(out, (int)outlen, F) != out) {
        return EOF;
    }
    eol = strchr(out, '\n');
    if (eol) {
        *eol = 0;
    }
    return 0;
}

static int fs_read(HSTREAM s, void * out, size_t outlen) {
    FILE * F = (FILE *)s.data;
    assert(outlen <= INT_MAX);
    if (!out) {
        return fseek(F, (long)outlen, SEEK_CUR);
    }
    if (outlen > 0) {
        size_t bytes = fread(out, 1, outlen, F);
        if (bytes != outlen) {
            if (feof(F)) {
                return EOF;
            }
            return ferror(F);
        }
    }
    return 0;
}

static int fs_writeln(HSTREAM s, const char * out) {
    FILE * F = (FILE *)s.data;
    int res = fputs(out, F);
    if (res < 0) {
        return res;
    }
    res = fputc('\n', F);
    if (res != '\n') {
        return res;
    }
    return 0;
}

static int fs_write(HSTREAM s, const void * out, size_t len) {
    if (len > 0) {
        FILE* F = (FILE*)s.data;
        size_t bytes;

        bytes = fwrite(out, 1, len, F);
        if (bytes != len) {
            return ferror(F);
        }
    }
    return 0;
}

static void fs_rewind(HSTREAM s) {
    FILE * F = (FILE *)s.data;
    rewind(F);
}

const stream_i filestream = {
    fs_writeln,
    fs_write,
    fs_readln,
    fs_read,
    fs_rewind
};

void fstream_init(struct stream * strm, FILE * F) {
    strm->api = &filestream;
    strm->handle.data = F;
}

void fstream_done(struct stream * strm) {
    FILE * F = (FILE *)strm->handle.data;
    fclose(F);
}
