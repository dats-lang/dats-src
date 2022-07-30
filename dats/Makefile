CC = gcc
CFLAGS = -Wall -Wextra -I.. -I../include

OBJ = dats.o
OBJ += parser.o 
OBJ += scanner.o
OBJ += wav.o
OBJ += semantic.o
OBJ += utils.o
OBJ += dquote.o
OBJ += genpcm.o
OBJ += write.o
OBJ += mixer.o

LDFLAGS = -L../libdsynth -ldsynth -L../libdfilter \
	-ldfilter -lm

include ../Makedefs

ifeq ($(ENABLE_DEBUG), yes)
	CFLAGS += -ggdb3 -O0
else
	CFLAGS += -O2
endif

ifeq ($(ENABLE_ASAN), yes)
	LDFLAGS += -fsanitize=address
	CFLAGS += -fsanitize=address
endif

ifeq ($(ENABLE_OPENMP), yes)
	CFLAGS += -fopenmp
	LDFLAGS += -fopenmp
endif

ifeq ($(OS), Windows_NT)
	LDFLAGS += -Wl,--stack,8388608 # 8 MiB
endif

ifneq ($(OS), Windows_NT)
	LDFLAGS += -ldl
endif

.PHONY : clean test indent

dats :$(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

version.h:
	xxd -i ../.git/refs/heads/master > version.h
	xxd -i ../.git/HEAD >> version.h

$(OBJ) :%.o:%.c $(wildcard *.h) version.h

clean: 
	rm dats $(OBJ) version.h

test : dats
	./dats ../test/t.dats
	./dats  ../test/terr.dats ../test/t.dats

install:  dats
	cp $^ $(PREFIX)/bin

uninstall: dats
	rm $(PREFIX)/bin/$^

indent :
	clang-format -i *.[ch]

