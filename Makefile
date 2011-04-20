CFLAGS = -Wall
CC = gcc
CUTEST = ..
INCLUDES = -I$(CUTEST)

all: storage_test

binarystore.o: binarystore.c binarystore.h storage.h
	$(CC) -c binarystore.c $(INCLUDES) $(CFLAGS)

textstore.o: textstore.c textstore.h storage.h
	$(CC) -c textstore.c $(INCLUDES) $(CFLAGS)

test_storage: binarystore.o textstore.o test_storage.c storage.h
	$(CC) -o test_storage textstore.o binarystore.o $(CUTEST)/cutest/CuTest.c test_storage.c $(INCLUDES) $(CFLAGS)

clean:
	@rm -f *.o *~ test_storage

