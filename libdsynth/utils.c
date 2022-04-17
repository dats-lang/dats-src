#include "utils.h"
#include "synth.h"

static uint32_t pcm_seek = 0;
static void write_block2(int16_t *pcm, void **args, bnr_t *bnr,
                         void(write_note)(int16_t *, void **, note_t *,
                                          uint32_t)) {
  for (uint8_t rep = 0; rep < bnr->block_repeat + 1; rep++)
    for (nr_t *n = bnr->nr; n != NULL; n = n->next) {
      if (n->type == SYM_NOTE) {
        for (note_t *note = n->note; note != NULL; note = note->next) {
          write_note(pcm, args, note, pcm_seek);
        }
      }
      if (n->type != SYM_BLOCK) {
        pcm_seek += n->length;
        continue;
      }
      write_block(pcm, args, n->block, write_note);
    }
}

void write_block(int16_t *pcm, void **args, bnr_t *bnr,
                 void(write_note)(int16_t *, void **, note_t *, uint32_t)) {
  write_block2(pcm, args, bnr, write_note);
  pcm_seek = 0;
}
