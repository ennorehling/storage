#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
#include "stream.h"
#include "filestream.h"
#include "memstream.h"

#include <CuTest.h>
#include <stdio.h>
#include <string.h>

static void setup_read(CuTest *tc, stream *strm) {
    CuAssertIntEquals(tc, 0, strm->api->write(strm->handle, "Hello\nWorld\n", 12));
}

static void test_read(CuTest * tc, const stream * strm) {
    char buf[16];

    buf[12]='\0';
    CuAssertIntEquals(tc, 0, strm->api->read(strm->handle, buf, 12));
    CuAssertIntEquals(tc, '\0', buf[12]);
    CuAssertStrEquals(tc, "Hello\nWorld\n", buf);
}

static void test_read_eof(CuTest * tc, const stream * strm) {
    char buf[16];

    buf[12]='\0';
    CuAssertIntEquals(tc, EOF, strm->api->read(strm->handle, buf, sizeof(buf)));
    CuAssertIntEquals(tc, '\0', buf[12]);
}

static void setup_readln(CuTest *tc, stream *strm) {
    const char *hello = "Hello World";

    CuAssertIntEquals(tc, 0, strm->api->writeln(strm->handle, hello));
    CuAssertIntEquals(tc, 0, strm->api->writeln(strm->handle, hello));
}

static void test_readln(CuTest * tc, const stream * strm) {
    char buf[16];
    const char * hello = "Hello World";

    CuAssertIntEquals(tc, 0, strm->api->readln(strm->handle, buf, sizeof(buf)));
    CuAssertStrEquals(tc, hello, buf);
    CuAssertIntEquals(tc, 0, strm->api->readln(strm->handle, buf, sizeof(buf)));
    CuAssertStrEquals(tc, hello, buf);
    CuAssertIntEquals(tc, EOF, strm->api->readln(strm->handle, buf, sizeof(buf)));
    CuAssertStrEquals(tc, hello, buf);
}

static void setup_readln_nocrlf(CuTest *tc, stream *strm) {
    const char *hello = "Hello World";

    CuAssertIntEquals(tc, 0, strm->api->write(strm->handle, hello, 11));
}

static void test_readln_nocrlf(CuTest * tc, const stream * strm) {
    char buf[16];
    const char *hello = "Hello World";

    CuAssertIntEquals(tc, 0, strm->api->readln(strm->handle, buf, sizeof(buf)));
    CuAssertStrEquals(tc, hello, buf);
}

static void test_filestream(CuTest * tc) {
    stream strm;
    const char *filename = "test.txt";

    if (filename) {
        FILE *F;

        F = fopen(filename, "wb+");
        if (F) {
            fstream_init(&strm, F);
            setup_read(tc, &strm);
            fstream_done(&strm);
        }
        F = fopen(filename, "rb");
        if (F) {
            fstream_init(&strm, F);
            test_read(tc, &strm);
            fstream_done(&strm);
        }
        F = fopen(filename, "rb");
        if (F) {
            fstream_init(&strm, F);
            test_read_eof(tc, &strm);
            fstream_done(&strm);
        }

        F = fopen(filename, "wb+");
        if (F) {
            fstream_init(&strm, F);
            setup_readln(tc, &strm);
            fstream_done(&strm);
        }
        F = fopen(filename, "rb+");
        if (F) {
            fstream_init(&strm, F);
            test_readln(tc, &strm);
            fstream_done(&strm);
        }

        F = fopen(filename, "wb+");
        if (F) {
            fstream_init(&strm, F);
            setup_readln_nocrlf(tc, &strm);
            fstream_done(&strm);
        }
        F = fopen(filename, "rb+");
        if (F) {
            fstream_init(&strm, F);
            test_readln_nocrlf(tc, &strm);
            fstream_done(&strm);
        }

        remove(filename);
    }
}

static void test_memstream(CuTest * tc) {
    stream strm;

    char ch;

    mstream_init(&strm);
    CuAssertIntEquals(tc, 0, strm.api->write(strm.handle, "123", 3));
    strm.api->rewind(strm.handle);
    CuAssertIntEquals(tc, 0, strm.api->read(strm.handle, &ch, 1));
    CuAssertIntEquals(tc, '1', ch);
    CuAssertIntEquals(tc, 0, strm.api->read(strm.handle, &ch, 1));
    CuAssertIntEquals(tc, '2', ch);
    CuAssertIntEquals(tc, 0, strm.api->read(strm.handle, &ch, 1));
    CuAssertIntEquals(tc, '3', ch);
    CuAssertIntEquals(tc, EOF, strm.api->read(strm.handle, &ch, 1));
    mstream_done(&strm);

    mstream_init(&strm);
    setup_read(tc, &strm);
    strm.api->rewind(strm.handle);
    test_read(tc, &strm);
    mstream_done(&strm);

    mstream_init(&strm);
    setup_read(tc, &strm);
    strm.api->rewind(strm.handle);
    test_read_eof(tc, &strm);
    mstream_done(&strm);

    mstream_init(&strm);
    setup_readln(tc, &strm);
    strm.api->rewind(strm.handle);
    test_readln(tc, &strm);
    mstream_done(&strm);

    mstream_init(&strm);
    setup_readln_nocrlf(tc, &strm);
    strm.api->rewind(strm.handle);
    test_readln_nocrlf(tc, &strm);
    mstream_done(&strm);
}

void add_suite_stream(CuSuite *suite)
{
    SUITE_ADD_TEST(suite, test_memstream);
    SUITE_ADD_TEST(suite, test_filestream);
}
