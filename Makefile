# I am not very good at Makefiles.

ifeq (,$(wildcard ../cutest))
CUTEST=.
else
CUTEST = ../cutest
INCLUDES += -I../cutest
endif

CFLAGS += -Wall -g
INCLUDES = -I. -I$(CUTEST)

all: bin/tests

bin:
	mkdir -p $@

test: bin/tests
	@bin/tests

bin/tests: tests.c storage.test.c stream.test.c \
  stream.h filestream.h memstream.h filestream.c memstream.c \
  storage.h binarystore.h binarystore.c textstore.h textstore.c \
$(CUTEST)/CuTest.c | bin
	$(CC) $(CFLAGS) $(INCLUDES) -lm -o $@ $^

clean:
	@rm -rf *~ bin
