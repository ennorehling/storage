#include "storage.h"
#include "binarystore.h"
#include "textstore.h"

#include <cutest/CuTest.h>
#include <stdio.h>
#include <string.h>

static void test_open_close(CuTest * tc)
{
}

int main(int argc, char ** argv)
{
  CuString *output = CuStringNew();
  CuSuite *suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_open_close);

  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
  return suite->failCount;
}
