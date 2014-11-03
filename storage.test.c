#include "storage.h"
#include "binarystore.h"
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

static void bin_open(storage * store, const char * filename, int mode) {
    const char *modes[] = { "r", "rb", "wb", "wb+" };
    FILE * F = fopen(filename, modes[mode]);
    binstore_init(store, F);
}
static void bin_close(storage * store) {
    binstore_done(store);
}
static factory bin_factory = {
    bin_open, bin_close
};

static void txt_open(storage * store, const char * filename, int mode) {
    const char *modes[] = { "r", "rb", "wb", "wb+" };
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
    char buffer[32];
    int i;
    float f;

    fac->open(&store, filename, IO_WRITE);
    CuAssertPtrNotNull(tc, store.handle.data);
    CuAssertTrue(tc, store.api->w_int(store.handle, 42) > 0);
    CuAssertTrue(tc, store.api->w_flt(store.handle, FLT_MAX) > 0);
    CuAssertTrue(tc, store.api->w_brk(store.handle) >= 0);
    CuAssertTrue(tc, store.api->w_str(store.handle, "Hello World") > 0);
    CuAssertTrue(tc, store.api->w_tok(store.handle, "gazebo") >= 0);
    fac->close(&store);

    fac->open(&store, filename, IO_READ);
    CuAssertPtrNotNull(tc, store.handle.data);

    CuAssertIntEquals(tc, 0, store.api->r_int(store.handle, &i));
    CuAssertIntEquals(tc, 42, i);

    CuAssertIntEquals(tc, 0, store.api->r_flt(store.handle, &f));
    CuAssertDblEquals(tc, FLT_MAX, f, FLT_MIN);
    CuAssertIntEquals(tc, 0, store.api->r_str(store.handle, buffer, 32));
    CuAssertStrEquals(tc, "Hello World", buffer);
    CuAssertIntEquals(tc, 0, store.api->r_tok(store.handle, buffer, 32));
    CuAssertStrEquals(tc, "gazebo", buffer);
    fac->close(&store);

    remove(filename);
}

static void test_read_write_bin(CuTest * tc) {
    test_read_write(tc, &bin_factory);
}

static void test_read_write_txt(CuTest * tc) {
    test_read_write(tc, &txt_factory);
}

void add_suite_storage(CuSuite *suite)
{
    SUITE_ADD_TEST(suite, test_read_write_bin);
    SUITE_ADD_TEST(suite, test_read_write_txt);
}
