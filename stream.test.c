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

    buf[12]='\0';
    sz = strm->api->read(strm->handle, buf, sizeof(buf));
    CuAssertIntEquals(tc, 0, buf[sz]);
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

static void test_read_write(CuTest *tc, stream *strm)
{
    char ch;

    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 3, strm->api->write(strm->handle, "123", 3));
    strm->api->rewind(strm->handle);
    CuAssertIntEquals(tc, 1, strm->api->read(strm->handle, &ch, 1));
    CuAssertIntEquals(tc, '1', ch);
    CuAssertIntEquals(tc, 1, strm->api->read(strm->handle, &ch, 1));
    CuAssertIntEquals(tc, '2', ch);
    CuAssertIntEquals(tc, 1, strm->api->read(strm->handle, &ch, 1));
    CuAssertIntEquals(tc, '3', ch);
}

static void test_filestream(CuTest * tc) {
    stream strm;

    fstream_init(&strm, fopen("test.txt", "w+"));
    test_read(tc, &strm);
    test_readln(tc, &strm);
    test_read_write(tc, &strm);
    fstream_done(&strm);
    remove("test.txt");
}

static void test_memstream(CuTest * tc) {
    stream strm;
    size_t sizes[10] = { 0, 1, 2, 4, 5, 6, 7, 8, 15, 16 };
    int s;

    for (s = 0; s < 10; ++s) {
        mstream_init_size(&strm, sizes[s]);
        test_read_write(tc, &strm);
        test_read(tc, &strm);
        test_readln(tc, &strm);
        mstream_done(&strm);
    }
}

static FILE *F;

static void fstreaminit(stream *strm) {
    F = fopen("foo.txt", "w");
    fstream_init(strm, F);
}

static void fstreamreset(stream *strm) {
    fstream_done(strm);

    F = fopen("foo.txt", "r");
    fstream_init(strm, F);
}

static void fstreamdone(stream *strm) {
    fstream_done(strm);
    remove("foo.txt");
}

static void mstreamreset(stream *strm) {
    strm->api->rewind(strm->handle);
}

static void stream_test(CuTest *tc, void (*stream_init)(struct stream * strm), void (*stream_reset)(struct stream * strm), void (*stream_done)(struct stream * strm)) {
    char buffer[127];
    stream strm;

    stream_init(&strm);

    sputs("bc", &strm);
    strm.api->rewind(strm.handle);
    sputs("a", &strm);
    sputs("", &strm);
    swrite("b", 1, 1, &strm);

    stream_reset(&strm);

    CuAssertIntEquals(tc, 0, strm.api->readln(strm.handle, buffer, sizeof(buffer)));
    CuAssertStrEquals(tc, "a", buffer);
    CuAssertIntEquals(tc, 0, strm.api->readln(strm.handle, buffer, sizeof(buffer)));
    CuAssertStrEquals(tc, "", buffer);
    CuAssertIntEquals(tc, 0, strm.api->readln(strm.handle, buffer, sizeof(buffer)));
    CuAssertStrEquals(tc, "b", buffer);
    CuAssertIntEquals(tc, -1, strm.api->readln(strm.handle, buffer, sizeof(buffer)));

    strm.api->rewind(strm.handle);
    CuAssertIntEquals(tc, 0, strm.api->readln(strm.handle, buffer, sizeof(buffer)));
    CuAssertStrEquals(tc, "a", buffer);
}


static void test_streams(CuTest *tc) {

    stream_test(tc, mstream_init, mstreamreset, mstream_done);

    stream_test(tc, fstreaminit, fstreamreset, fstreamdone);
}

static void test_memstream_page(CuTest *tc) {
    char outbuffer[21];
    char inbuffer[21];
    stream strm;
    int i;

    for (i = 0; i < sizeof(outbuffer); ++i) {
        outbuffer[i] = 'a' + (i%26);
        inbuffer[i] = '$';
    }
    outbuffer[sizeof(outbuffer)-1] = 0;

    mstream_init_size(&strm, 6);

    swrite(outbuffer, 1, sizeof(outbuffer)-1, &strm);

    mstreamreset(&strm);

    CuAssertIntEquals(tc, 0, strm.api->readln(strm.handle, inbuffer, sizeof(inbuffer)));
    CuAssertStrEquals(tc, outbuffer, inbuffer);

    mstream_done(&strm);
}

void add_suite_stream(CuSuite *suite)
{
    SUITE_ADD_TEST(suite, test_memstream);
    SUITE_ADD_TEST(suite, test_filestream);
    SUITE_ADD_TEST(suite, test_streams);
    SUITE_ADD_TEST(suite, test_memstream_page);
}
