# I am not very good at Makefiles.

ifndef CUTEST
CUTEST = ../cutest
endif

CFLAGS += -Wall -g
INCLUDES = -I. -I$(CUTEST)

all: tests

bin:
	mkdir -p $@

tests: bin/tests
	@bin/tests

bin/tests: tests.c storage.test.c stream.test.c \
  stream.h filestream.h memstream.h filestream.c memstream.c \
  storage.h binarystore.h binarystore.c textstore.h textstore.c \
$(CUTEST)/CuTest.c | bin
	$(CC) $(CFLAGS) $(INCLUDES) -lm -o $@ $^

clean:
	@rm -rf *~ bin
