#include <stdint.h>
#include <stdlib.h>

#include "synth.h"
#include "log.h"

static int synth(const symrec_t *const staff, pcm16_t *const pcm_ctx);
 
/* clang-format off */
static DSOption options[] = {
    {.option_name = NULL}

};
/* clang-format on */

static void free_string_options(void) {
  for (int i = 0; options[i].option_name != NULL; i++) {
    if (options[i].type != DSOPTION_STRING) {
      options[i].value.intv = 0;
      options[i].value.floatv = 0.0;
      continue;
    }
    free(options[i].value.strv);
  }
}

static int synth(const symrec_t *const staff, pcm16_t *const pcm_ctx) {
//  int16_t *pcm = pcm_ctx->pcm;
  uint32_t tnb_samples = staff->value.staff.nb_samples + 1024;
  int16_t *pcm = calloc(tnb_samples, sizeof(int16_t));
  if (pcm == NULL){
    DSYNTH_LOG("no mem\n");
    return 1;
  }

  uint32_t total = 0;
  for (nr_t *n = staff->value.staff.nr; n != NULL; n = n->next) {
    if (n->type == SYM_NOTE) {
      for (note_t *nn = n->note; nn != NULL; nn = nn->next) {
        int16_t wavetable[(int)(44100.0 / nn->frequency)];
        for (int i = 0; i < (int)(44100.0 / nn->frequency); i++)
          wavetable[i] = rand();
        int16_t prev = 0;
        uint32_t cur = 0;
        for (uint32_t i = 0; i < nn->duration; i++) {
          wavetable[cur] = ((wavetable[cur]/2) + (prev / 2));
          pcm[total + i] +=
              (int16_t)
              /* simple linear attack and linear decay filter */
              (double)wavetable[cur] *
              (i < (uint32_t)nn->attack
                   ? (double)i / nn->attack
                   : (i > nn->duration - (uint32_t)nn->release
                          ? (-(double)i + (double)(nn->duration)) / nn->release
                          : 1.0));
          prev = wavetable[cur];
          cur++;
          cur %= (int)(44100.0 / nn->frequency);
        }
        prev = 0;
        cur = 0;
      }
    }
    total += n->length;
    if ((total % 44100) < 1000) {
      printf("\r[s_kpa] %d/%d", total, staff->value.staff.nb_samples);
      fflush(stdout);
    }
  }
  putchar('\n');
  for (DSOption *ctx = options; ctx->option_name != NULL; ctx++) {
    printf("[s_kpa] %s ", ctx->option_name);
    switch (ctx->type) {
    case DSOPTION_FLOAT:
      printf("%f", ctx->value.floatv);
      break;
    case DSOPTION_INT:
      printf("%d", ctx->value.intv);
      break;
    case DSOPTION_STRING:
      printf("%s", ctx->value.strv != NULL ? ctx->value.strv : " ");
      break;
    }
    putchar('\n');
  }
  pcm_ctx->pcm = pcm;
  pcm_ctx->nb_samples = tnb_samples;
  pcm_ctx->play_end = tnb_samples;
  free_string_options();
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
