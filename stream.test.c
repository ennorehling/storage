#include "stream.h"
#include "filestream.h"
#include "memstream.h"

#include <CuTest.h>
#include <stdio.h>

static void test_read_write(CuTest * tc, const stream * strm) {
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

static void test_read_write_memory(CuTest * tc) {
    stream strm;

    mstream_init(&strm);
    test_read_write(tc, &strm);
    mstream_done(&strm);
}

static void test_read_write_file(CuTest * tc) {
    stream strm;

    fstream_init(&strm, fopen("test.txt", "w+b"));
    test_read_write(tc, &strm);
    fstream_done(&strm);
}

void add_suite_stream(CuSuite *suite)
{
    SUITE_ADD_TEST(suite, test_read_write_file);
    SUITE_ADD_TEST(suite, test_read_write_memory);
}
