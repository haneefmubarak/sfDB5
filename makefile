# Basic C-related flags
## Compiler (use gcc if possible, else clang)
CC = $(shell (command -v gcc >/dev/null 2>&1 && echo gcc) || echo failure)
ifeq ($(CC),failure)
	$(error 'gcc' is required to compile sfDB5.)
endif

## Generic Flags
CFLAGS = -O2 -march=native -mtune=native -Wall -Wextra
LFLAGS =

## Extra Flags
CFLAGS += -pthread

# OS Dependent Flags
UNAME_S = $(shell uname -s)
## gcc OSX assembly fix
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -Wa,-q
endif

# move into source directory
SOURCE ?= ./src
CD = cd $(SOURCE);

all:	sfDB5

clean:
	rm -rvf sfDB5 src/*.o

sfDB5:	src/sfDB5.o
	$(CC) $(CFLAGS) $(LFLAGS) -o sfDB5 src/sfDB5.o

test:	sfDB5
	./sfDB5 --test || (echo; echo Testing reported failure)

src/sfDB5.o:	src/sfDB5.h src/sfDB5.c
	$(CD) $(CC) $(CFLAGS) -c sfDB5.c
