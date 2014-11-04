#include "stream.h"
#include "filestream.h"
#include <stdio.h>
#include <string.h>

static int fs_readln(HSTREAM s, char * out, size_t outlen) {
    char * eol;
    FILE * F = (FILE *)s.data;

    if (!out) {
        return fseek(F, outlen, SEEK_CUR);
    }
    if (fgets(out, outlen, F) != out) {
        return EOF;
    }
    eol = strchr(out, '\n');
    if (eol) {
        *eol = 0;
    }
    return 0;
}

static size_t fs_read(HSTREAM s, void * out, size_t outlen) {
    FILE * F = (FILE *)s.data;
    size_t result = fread(out, sizeof(char), outlen, F);
    return result;
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
    FILE * F = (FILE *)s.data;
    int res = fwrite(out, 1, len, F);
    return res;
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
