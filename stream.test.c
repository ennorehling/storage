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

static void test_read(CuTest * tc, const stream * strm) {
    char buf[16];

    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 0, strm->api->writeln(strm->handle, "Hello"));
    CuAssertIntEquals(tc, 0, strm->api->writeln(strm->handle, "World"));

    strm->api->rewind(strm->handle);

    buf[12]='\0';
    CuAssertIntEquals(tc, EOF, strm->api->read(strm->handle, buf, sizeof(buf)));
    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 0, strm->api->read(strm->handle, buf, 12));
    CuAssertIntEquals(tc, '\0', buf[12]);
    CuAssertStrEquals(tc, "Hello\nWorld\n", buf);
}

static void test_readln(CuTest * tc, const stream * strm) {
    char buf[64];
    const char * hello = "Hello World";

    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 0, strm->api->writeln(strm->handle, hello));
    CuAssertIntEquals(tc, 0, strm->api->writeln(strm->handle, hello));

    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 0, strm->api->readln(strm->handle, buf, sizeof(buf)));
    CuAssertStrEquals(tc, hello, buf);
    CuAssertIntEquals(tc, 0, strm->api->readln(strm->handle, buf, sizeof(buf)));
    CuAssertStrEquals(tc, hello, buf);
    CuAssertIntEquals(tc, EOF, strm->api->readln(strm->handle, buf, sizeof(buf)));
    CuAssertStrEquals(tc, hello, buf);
}

static void test_readln_unterminated(CuTest * tc, const stream * strm) {
    char buf[64];
    const char * hello = "Hello World";

    CuAssertIntEquals(tc, 0, strm->api->write(strm->handle, hello, strlen(hello)));

    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 0, strm->api->readln(strm->handle, buf, sizeof(buf)));
    CuAssertStrEquals(tc, hello, buf);
}

static void test_read_write(CuTest *tc, stream *strm)
{
    char ch;

    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 0, strm->api->write(strm->handle, "123", 3));
    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 0, strm->api->read(strm->handle, &ch, 1));
    CuAssertIntEquals(tc, '1', ch);
    CuAssertIntEquals(tc, 0, strm->api->read(strm->handle, &ch, 1));
    CuAssertIntEquals(tc, '2', ch);
    CuAssertIntEquals(tc, 0, strm->api->read(strm->handle, &ch, 1));
    CuAssertIntEquals(tc, '3', ch);
    CuAssertIntEquals(tc, EOF, strm->api->read(strm->handle, &ch, 1));
}

static void test_filestream(CuTest * tc) {
    stream strm;

    fstream_init(&strm, fopen("test.txt", "wbT+"));
    test_read(tc, &strm);
    fstream_done(&strm);

    fstream_init(&strm, fopen("test.txt", "wbT+"));
    test_readln(tc, &strm);
    fstream_done(&strm);

    fstream_init(&strm, fopen("test.txt", "wbT+"));
    test_read_write(tc, &strm);
    fstream_done(&strm);
    remove("test.txt");
}

static void test_memstream(CuTest * tc) {
    stream strm;

    mstream_init(&strm);
    test_read_write(tc, &strm);
    mstream_done(&strm);

    mstream_init(&strm);
    test_read(tc, &strm);

    mstream_init(&strm);
    mstream_done(&strm);

    mstream_init(&strm);
    test_readln(tc, &strm);
    mstream_done(&strm);

    mstream_init(&strm);
    test_readln_unterminated(tc, &strm);
    mstream_done(&strm);
}

void add_suite_stream(CuSuite *suite)
{
    SUITE_ADD_TEST(suite, test_memstream);
    SUITE_ADD_TEST(suite, test_filestream);
}
