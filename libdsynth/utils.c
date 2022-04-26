#include "utils.h"
#include "synth.h"
#include <inttypes.h>

static uint32_t pcm_seek = 0;
// static char space[42] = "                                          ";
static uint32_t scope = 0;
static int16_t *pcm = NULL;

static void write_block2(void **args, bnr_t *bnr,
                         void(write_note)(int16_t *, void **, note_t *,
                                          uint32_t)) {
  for (uint8_t rep = 0; rep < bnr->block_repeat + 1; rep++)
    for (nr_t *n = bnr->nr; n != NULL; n = n->next) {
      switch (n->type) {
      case SYM_NOTE:
        // fprintf(stderr, "%*.snote at %p #%"PRIu8 " ", scope, space, bnr,
        // bnr->block_id);
        for (note_t *note = n->note; note != NULL; note = note->next) {
          // fprintf(stderr, "n ");
          write_note(pcm, args, note, pcm_seek);
        }
        // putchar('\n');
        pcm_seek += n->length;
        continue;
      case SYM_REST:
        // fprintf(stderr, "%*.srest at %p #%"PRIu8 " ", scope, space, bnr,
        // bnr->block_id);
        pcm_seek += n->length;
        continue;
      case SYM_BLOCK:
        scope++;
        write_block2(args, n->block, write_note);
        scope--;
        continue;
      }
    }
}

void write_block(int16_t *ipcm, void **args, bnr_t *bnr,
                 void(write_note)(int16_t *, void **, note_t *, uint32_t)) {
  pcm = ipcm;
  write_block2(args, bnr, write_note);
  pcm_seek = 0;
  scope = 0;
  pcm = NULL;
}
