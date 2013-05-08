# I am not very good at Makefiles.

ifndef CUTEST
CUTEST = ../cutest
endif

CFLAGS += -Wall -O3
INCLUDES = -I. -I$(CUTEST)

all: tests

bin:
	mkdir -p $@

tests: bin/tests
	@bin/tests

bin/tests: tests.c \
storage.test.c stream.test.c stream.h filestream.h memstream.h filestream.c memstream.c binarystore.c textstore.c \
$(CUTEST)/CuTest.c | bin
	$(CC) $(CFLAGS) $(INCLUDES) -lm -o $@ $^

clean:
	@rm -rf *~ bin
