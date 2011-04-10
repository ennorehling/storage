#include "storage.h"
#include "binarystore.h"
#include "textstore.h"

#include <cutest/CuTest.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

const storage * api;

static void test_open_close(CuTest * tc)
{
  FILE * F;
  HSTORAGE store;
  
  F = fopen("test.dat", "wb");
  store = api->begin(F, IO_WRITE);
  CuAssertPtrNotNull(tc, store);
  CuAssertIntEquals(tc, 0, api->end(store));

  F = fopen("test.dat", "rb");
  store = api->begin(F, IO_READ);
  CuAssertPtrNotNull(tc, store);
  CuAssertIntEquals(tc, 0, api->end(store));
}

static void test_read_write(CuTest * tc)
{
  FILE * F;
  HSTORAGE store;
  char buffer[32];
  
  F = fopen("test.dat", "wb");
  store = api->begin(F, IO_WRITE);
  CuAssertPtrNotNull(tc, store);
  CuAssertTrue(tc, api->w_int(store, 42)>0);
  CuAssertTrue(tc, api->w_flt(store, FLT_MAX)>0);
  CuAssertTrue(tc, api->w_brk(store)>=0);
  CuAssertTrue(tc, api->w_str(store, "Hello World")>0);
  CuAssertTrue(tc, api->w_tok(store, "gazebo")>=0);
  CuAssertIntEquals(tc, 0, api->end(store));
  fclose(F);

  F = fopen("test.dat", "rb");
  store = api->begin(F, IO_READ);
  CuAssertPtrNotNull(tc, store);
  CuAssertIntEquals(tc, 42, api->r_int(store));
  CuAssertDblEquals(tc, FLT_MAX, api->r_flt(store), FLT_MIN);
  CuAssertIntEquals(tc, 0, api->r_str(store, buffer, 32));
  CuAssertStrEquals(tc, "Hello World", buffer);
  CuAssertIntEquals(tc, 0, api->r_tok(store, buffer, 32));
  CuAssertStrEquals(tc, "gazebo", buffer);
  CuAssertIntEquals(tc, 0, api->end(store));
  fclose(F);
}

int main(int argc, char ** argv)
{
  CuString *output = CuStringNew();
  CuSuite *suite = CuSuiteNew();

  SUITE_ADD_TEST(suite, test_open_close);
  SUITE_ADD_TEST(suite, test_read_write);

  api = &text_store;
  CuSuiteRun(suite);

  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
  return suite->failCount;
}
