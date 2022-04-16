#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef DATS_DETECT_MEM_LEAK
#include "memory-leak-detector/leak_detector.h"
#endif
#include "synth.h"

extern const DSSynth *get_dsynth_by_name(const char *name);

static DSOption options[] = {
    {DSOPTION_STRING, "command", "Command to parse", {.strv = NULL}},
    {.option_name = NULL}

};

static void reset_options_to_default(void) {
  for (int i = 0; options[i].option_name != NULL; i++) {
    if (options[i].type != DSOPTION_STRING) {
      options[i].value.floatv = 0;
      options[i].value.floatv = 0.0;
      continue;
    }
    free(options[i].value.strv);
  }
}

static track_t *synth(const symrec_t *staff) {
  int16_t *pcm = calloc(sizeof(int16_t), (size_t)staff->value.staff.nb_samples);
  track_t *pcm_ctx = malloc(sizeof(track_t));
  if (pcm_ctx == NULL || pcm == NULL)
    return NULL;

  uint32_t total = 0;
  for (nr_t *n = staff->value.staff.nr; n != NULL; n = n->next) {
    if (n->type == SYM_NOTE) {
      for (note_t *nn = n->note; nn != NULL; nn = nn->next) {
        for (uint32_t i = 0; i < n->length; i++) {
          pcm[total + i] +=
              (int16_t)((double)nn->volume *
                        sin(2.0 * M_PI * nn->frequency * (double)i / 44100.0));
        }
      }
    }
    total += n->length;
    if ((total % 44100) < 1000) {
      printf("\r[s_synth] %d/%d", total, staff->value.staff.nb_samples);
      fflush(stdout);
    }
  }
  putchar('\n');
  for (DSOption *ctx = options; ctx->option_name != NULL; ctx++) {
    printf("[s_synth] %s ", ctx->option_name);
    switch (ctx->type) {
    case DSOPTION_FLOAT:
      printf("%f", ctx->value.floatv);
      break;
    case DSOPTION_FLOAT:
      printf("%d", ctx -.value.floatv);
      break;
    case DSOPTION_STRING:
      printf("%s", ctx->value.strv != NULL ? ctx->value.strv : " ");
      break;
    }
    putchar('\n');
  }
  pcm_ctx->numsamples = staff->value.staff.nb_samples;
  pcm_ctx->pcm = pcm;
  pcm_ctx->next = NULL;
  reset_options_to_default();
  return pcm_ctx;
}

/* clang-format off */
DSSynth ss_synth = {
  .name = "synth",
  .description = "A multi-purpose synthesizer",
  .options = options,
  .synth = &synth
};
/* clang-format on */
