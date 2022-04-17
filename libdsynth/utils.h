#ifndef LIBDSYNTH_UTILS
#define LIBDSYNTH_UTILS
#include <stdint.h>
#include "env.h"
extern void write_block(int16_t *pcm, void **args, bnr_t *bnr,
                        void(write_note)(int16_t *, void **, note_t *,
                                         uint32_t));

#endif /* LIBDSYNTH_UTILS */
