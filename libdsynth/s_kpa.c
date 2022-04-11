#include <stdint.h>
#include <stdlib.h>

#include "synth.h"
#include "log.h"
#include "utils.h"

static int synth(const symrec_t *const staff, pcm16_t *const pcm_ctx);
 
/* clang-format off */
static DSOption options[] = {
    {.option_name = NULL}

};
/* clang-format on */

static void reset_options_to_default(void) {
  for (int i = 0; options[i].option_name != NULL; i++) {
    if (options[i].type != DSOPTION_STRING) {
      options[i].value.floatv = 0.0;
      continue;
    }
    //free(options[i].value.strv);
  }
}

static void write_note(int16_t *pcm, void *args, note_t *note, uint32_t seek_pcm){
  int16_t wavetable[(int)(44100.0 / note->frequency)];
  for (int i = 0; i < (int)(44100.0 / note->frequency); i++)
    wavetable[i] = rand();
  int16_t prev = 0;
  uint32_t cur = 0;
  for (uint32_t i = 0; i < note->duration + 1024; i++) {
    wavetable[cur] = ((wavetable[cur]/2) + (prev / 2));
    pcm[seek_pcm + i] +=
        (int16_t)
        /* simple linear attack and linear decay filter */
        (double)wavetable[cur] *
        (i < (uint32_t)note->attack
             ? (double)i / note->attack
             : (i > note->duration - (uint32_t)note->release
                    ? (-(double)i + (double)(note->duration)) / note->release
                    : 1.0));
    prev = wavetable[cur];
    cur++;
    cur %= (int)(44100.0 / note->frequency);
  }

}

static int synth(const symrec_t *const staff, pcm16_t *const pcm_ctx) {
  uint32_t tnb_samples = staff->value.staff.nb_samples + 1024;
  int16_t *pcm = calloc(tnb_samples, sizeof(int16_t));
  if (pcm == NULL){
    DSYNTH_LOG("no mem\n");
    return 1;
  }

  write_block(pcm, NULL, staff->value.staff.bnr, write_note);
  
  for (DSOption *ctx = options; ctx->option_name != NULL; ctx++) {
    printf("[s_kpa] %s ", ctx->option_name);
    switch (ctx->type) {
    case DSOPTION_FLOAT:
      printf("%f", ctx->value.floatv);
      break;
    case DSOPTION_STRING:
      printf("%s", ctx->value.strv != NULL ? ctx->value.strv : " ");
      break;
    }
    putchar('\n');
  }
  pcm_ctx->pcm = pcm;
  pcm_ctx->nb_samples = tnb_samples;
  pcm_ctx->play_end = staff->value.staff.nb_samples;
  reset_options_to_default();
  return 0;
}

/* clang-format off */
DSSynth ss_kpa = {
  .name = "kpa",
  .description = "A Karplus-Strong synthesizer",
  .options = options,
  .synth = &synth
};
/* clang-format on */
