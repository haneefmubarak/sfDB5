# Basic C-related flags
## Compiler (use gcc if possible, else clang)
CC = $(shell (command -v gcc >/dev/null 2>&1 && echo gcc) || (command -v clang >/dev/null 2>&1 && echo clang) || echo failure)
ifeq ($(CC),failure)
	$(error 'gcc' or 'clang' are required to compile sfDB5. gcc is preferred.)
endif
## Generic Flags
CFLAGS = -O2 -march=native -mtune=native -Wall -Wextra
LFLAGS =

## Extra Flags
CFLAGS += -pthread

# OS Dependent Flags
UNAME_S = $(shell uname -s)
## gcc OSX assembly fix
ifeq ($(UNAME_S),Linux)
	CFLAGS += -Wa,-q
endif

all:	sfDB5

clean:
	rm -rvf sfDB5 dev sfDB5.o drive.o

sfDB5:	sfDB5.h sfDB5.c drive.h drive.c
	$(CC) $(CFLAGS) $(LFLAGS) -o sfDB5 sfDB5.c drive.c

test:	sfDB5
	./sfDB5 --test || (echo; echo Testing reported failure)

dev:	sfDB5.o drive.o
	$(CC) $(CFLAGS) $(LFLAGS) -o sfDB5 sfDB5.o drive.o
	touch dev

sfDB5.o:	sfDB5.h sfDB5.c
	$(CC) $(CFLAGS) -c sfDB5.c

drive.o:	sfDB5.h drive.h drive.c
	$(CC) $(CFLAGS) -c drive.c
