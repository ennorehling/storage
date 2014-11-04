#include "stream.h"
#include "filestream.h"
#include "memstream.h"

#include <CuTest.h>
#include <stdio.h>

static void test_read(CuTest * tc, const stream * strm) {
    char buf[16];
    size_t sz;

    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 0, strm->api->writeln(strm->handle, "Hello"));
    CuAssertIntEquals(tc, 0, strm->api->writeln(strm->handle, "World"));

    strm->api->rewind(strm->handle);

    sz = strm->api->read(strm->handle, buf, sizeof(buf));
    buf[sz] = 0;
    CuAssertStrEquals(tc, "Hello\nWorld\n", buf);
    CuAssertIntEquals(tc, 12, sz);
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

static void test_read_memory(CuTest * tc) {
    stream strm;

    mstream_init(&strm);
    test_read(tc, &strm);
    mstream_done(&strm);
}

static void test_read_file(CuTest * tc) {
    stream strm;

    fstream_init(&strm, fopen("test.txt", "w+b"));
    test_read(tc, &strm);
    fstream_done(&strm);
    remove("test.txt");
}

static void test_readln_memory(CuTest * tc) {
    stream strm;

    mstream_init(&strm);
    test_readln(tc, &strm);
    mstream_done(&strm);
}

static void test_readln_file(CuTest * tc) {
    stream strm;

    fstream_init(&strm, fopen("test.txt", "w+b"));
    test_readln(tc, &strm);
    fstream_done(&strm);
    remove("test.txt");
}

void add_suite_stream(CuSuite *suite)
{
    SUITE_ADD_TEST(suite, test_read_file);
    SUITE_ADD_TEST(suite, test_readln_file);
    SUITE_ADD_TEST(suite, test_read_memory);
    SUITE_ADD_TEST(suite, test_readln_memory);
}
