#include "stream.h"
#include <CuTest.h>

static void test_read_write(CuTest * tc) {
    HSTREAM s;
    
}

void add_suite_stream(CuSuite *suite)
{
    SUITE_ADD_TEST(suite, test_read_write);
}
