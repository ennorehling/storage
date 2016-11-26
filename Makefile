# I am not very good at Makefiles.

ifeq (,$(wildcard ../cutest))
CUTEST=.
else
CUTEST = ../cutest
INCLUDES += -I../cutest
endif

CFLAGS += -Wall -g -Werror -Wpointer-arith -Wno-char-subscripts
CFLAGS += -Wno-unknown-pragmas -Wstrict-prototypes -Wno-long-long
INCLUDES = -I. -I$(CUTEST)

all: bin/tests

bin obj:
	mkdir -p $@

obj/%.o: %.c | obj
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDES)

obj/test_%.o: test_%.c %.h | obj
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDES)

test: bin/tests
	@bin/tests

bin/tests: obj/tests.o obj/storage.test.o obj/stream.test.o \
  obj/filestream.o obj/memstream.o obj/binarystore.o \
  obj/textstore.o obj/CuTest.o | bin
	$(CC) $(CFLAGS) $(INCLUDES) -lm $^ -o $@

clean:
	@rm -rf *~ bin obj
