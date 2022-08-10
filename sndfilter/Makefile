CC = gcc
CFLAGS = -Wall -Wextra -fPIC 

include ../Makedefs

ifeq ($(ENABLE_DEBUG), yes)
	CFLAGS += -ggdb3 -O0 
	LDFLAGS += -fsanitize=address
else
	CFLAGS += -O2
endif

ifeq ($(ENABLE_OPENMP), yes)
	CFLAGS += -fopenmp
	LDFLAGS += -fopenmp
endif


OBJ  = mem.o
OBJ += snd.o
OBJ += wav.o
OBJ += biquad.o
OBJ += compressor.o
OBJ += reverb.o

libsndfilter.a : $(OBJ)
	$(AR) rcs $@ $^
	chmod +x $@

$(OBJ) : %.o : %.c

clean :
	rm $(OBJ) libsndfilter.a
