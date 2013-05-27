#include <CuTest.h>
#include <stdio.h>
#include <string.h>

void add_suite_storage(CuSuite *suite);
void add_suite_stream(CuSuite *suite);

int main(void)
{
  CuString *output = CuStringNew();
  CuSuite *suite = CuSuiteNew();

  add_suite_storage(suite);
  add_suite_stream(suite);

  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
  return suite->failCount;
}

