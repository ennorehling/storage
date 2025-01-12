#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
#include "storage.h"
#include "binarystore.h"
#include "filestream.h"
#include "stream.h"
#include "textstore.h"

#include <CuTest.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

typedef struct factory {
    void(*open)(storage *, const char * filename, int);
    void(*close)(storage *);
} factory;

#define IO_READ 0x01
#define IO_WRITE 0x02
static const char *modes[] = { "", "r", "w", "w+" };

static stream strm;

static void bin_open(storage * store, const char * filename, int mode) {
    FILE * F = fopen(filename, modes[mode]);
    
    fstream_init(&strm, F);
    binstore_init(store, &strm);
}

static void bin_close(storage * store) {
    binstore_done(store);
    fstream_done(&strm);
}
static factory bin_factory = {
    bin_open, bin_close
};

static void txt_open(storage * store, const char * filename, int mode) {
    FILE * F = fopen(filename, modes[mode]);
    txtstore_init(store, F);
}
static void txt_close(storage * store) {
    txtstore_done(store);
}
static factory txt_factory = {
    txt_open, txt_close
};

static void test_read_write(CuTest * tc, factory * fac)
{
    const char * filename = "test.dat";
    storage store;
    char buffer[TOKEN_MAXSIZE];
    int i;
    float f;

    fac->open(&store, filename, IO_WRITE);
    CuAssertPtrNotNull(tc, store.handle.data);
    CuAssertIntEquals(tc, 0, store.api->w_int(store.handle, 42));
    CuAssertIntEquals(tc, 0, store.api->w_flt(store.handle, FLT_MAX));
    CuAssertIntEquals(tc, 0, store.api->w_brk(store.handle));
    CuAssertIntEquals(tc, 0, store.api->w_str(store.handle, "Hello World"));
    CuAssertIntEquals(tc, 0, store.api->w_tok(store.handle, "gazebo"));
    CuAssertIntEquals(tc, 0, store.api->w_tok(store.handle, "ja"));
    fac->close(&store);

    fac->open(&store, filename, IO_READ);
    CuAssertPtrNotNull(tc, store.handle.data);

    CuAssertIntEquals(tc, 0, store.api->r_int(store.handle, &i));
    CuAssertIntEquals(tc, 42, i);

    CuAssertIntEquals(tc, 0, store.api->r_flt(store.handle, &f));
    CuAssertDblEquals(tc, FLT_MAX, f, FLT_MIN);
    CuAssertIntEquals(tc, 0, store.api->r_str(store.handle, buffer, TOKEN_MAXSIZE));
    CuAssertStrEquals(tc, "Hello World", buffer);
    CuAssertIntEquals(tc, 0, store.api->r_tok(store.handle, buffer, TOKEN_MAXSIZE));
    CuAssertStrEquals(tc, "gazebo", buffer);
    CuAssertIntEquals(tc, 0, store.api->r_tok(store.handle, buffer, TOKEN_MAXSIZE));
    CuAssertStrEquals(tc, "ja", buffer);
    CuAssertIntEquals(tc, EOF, store.api->r_tok(store.handle, buffer, TOKEN_MAXSIZE));
    fac->close(&store);

    (void) remove(filename);
}

static void test_read_write_bin(CuTest * tc) {
    test_read_write(tc, &bin_factory);
}

static void test_read_write_txt(CuTest * tc) {
    test_read_write(tc, &txt_factory);
}

static void test_read_write_ints(CuTest * tc) {
    const char *filename = "test.dat";
    storage store;
    int i;

    remove(filename);

    bin_open(&store, filename, IO_WRITE);
    CuAssertIntEquals(tc, 0, WRITE_INT(&store, 355747));
    CuAssertIntEquals(tc, 0, WRITE_INT(&store, 416957));
    bin_close(&store);

    bin_open(&store, filename, IO_READ);
    CuAssertIntEquals(tc, 0, READ_INT(&store, &i));
    CuAssertIntEquals(tc, 355747, i);
    CuAssertIntEquals(tc, 0, READ_INT(&store, &i));
    CuAssertIntEquals(tc, 416957, i);
    bin_close(&store);

    remove(filename);
}

void add_suite_storage(CuSuite *suite)
{
    SUITE_ADD_TEST(suite, test_read_write_bin);
    SUITE_ADD_TEST(suite, test_read_write_ints);
    SUITE_ADD_TEST(suite, test_read_write_txt);
}
