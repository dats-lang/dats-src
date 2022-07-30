CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I.. -I../include

include ../Makedefs

ifeq ($(ENABLE_DEBUG), yes)
	CFLAGS += -ggdb3 -O0
else
	CFLAGS += -O2
endif

ifeq ($(ENABLE_ASAN), yes)
	CFLAGS += -fsanitize=address
endif

ifeq ($(ENABLE_OPENMP), yes)
	CFLAGS += -fopenmp
	LDFLAGS += -fopenmp
endif

ifeq ($(OS), Windows_NT)
	LDFLAGS += -Wl,--out-implib,libdsynth.a
	TARGET = libdsynth.dll
else
	TARGET = libdsynth.so
endif

MISC = sf2.c allsynth.c utils.c

S_SYNTHS += s_kpa.c
S_SYNTHS += s_sin.c 
S_SYNTHS += s_square.c
#S_SYNTHS += s_synth.c
S_SYNTHS += s_sf2.c

OBJ := $(patsubst %.c, %.o, $(S_SYNTHS) $(MISC))

$(TARGET): $(OBJ)
	$(CC) -shared $^ -lm -lfluidsynth -o $@ $(LDFLAGS)

$(OBJ): %.o: %.c $(wildcard *.h)

.PHONY: indent
clean: libdsynth.so $(OBJ)
	rm $^

install: libdsynth.so
	cp $^ $(PREFIX)/lib

uninstall: libdsynth.so
	rm $(PREFIX)/lib/$^
indent:
	clang-format -i *.[ch]
