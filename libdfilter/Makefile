include ../Makedefs

LDFLAGS += -lm -L../sndfilter -lsndfilter

CFLAGS += -fPIC

ifeq ($(ENABLE_DEBUG), yes)
	CFLAGS += -ggdb3 -O0
else
	CFLAGS += -O3
endif

ifeq ($(ENABLE_ASAN), yes)
	LDFLAGS += -fsanitize=address
endif

ifeq ($(ENABLE_OPENMP), yes)
	CFLAGS += -fopenmp
	LDFLAGS += -fopenmp
endif

ifeq ($(OS), Windows_NT)
	LDFLAGS += -Wl,--out-implib,libdfilter.a
	TARGET = libdfilter.dll
else
	TARGET = libdfilter.so
endif

F_FILTER += f_reverb.c

OBJ := $(patsubst %.c, %.o, $(F_FILTER) allfilter.c)

$(TARGET): $(OBJ)
	$(CC) -shared $^ -o $@ $(LDFLAGS)

$(OBJ): %.o: %.c $(wildcard *.h)

.PHONY: indent
clean: libdfilter.so $(OBJ)
	rm $^

install: libdfilter.so
	cp $^ $(PREFIX)/lib

uninstall: libdfilter.so
	rm $(PREFIX)/lib/$^
indent:
	clang-format -i *.[ch]
