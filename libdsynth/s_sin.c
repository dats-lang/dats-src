#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "synth.h"
#include "utils.h"

/* clang-format off */
static DSOption options[] = {
   {DSOPTION_FLOAT, "vibrato_frequency", "Vibrato frequency", {.floatv = 0}},
   {DSOPTION_FLOAT, "vibrato_magnitude", "Vibrato magnitude", {.floatv = 0}},
   {DSOPTION_FLOAT, "attack_type", "Attack type (linear=0, exponential=1)", {.floatv = 0}},
   {DSOPTION_FLOAT, "decay_type", "Decay type (linear=0, exponential=1)", {.floatv = 0}},
   {DSOPTION_FLOAT, "sustain_type", "Sustain type (linear=0, exponential=1)", {.floatv = 0}},
   {DSOPTION_FLOAT, "release_type", "Release type (linear=0, exponential=1)", {.floatv = 0}},
   {DSOPTION_FLOAT, "attack_exp_coeff", "Attack exponential coefficiant", {.floatv = 0.0}},
   {DSOPTION_FLOAT, "decay_exp_coeff", "Attack exponential coefficiant", {.floatv = 0.0}},
   {DSOPTION_FLOAT, "sustain_exp_coeff", "Attack exponential coefficiant", {.floatv = 0.0}},
   {DSOPTION_FLOAT, "release_exp_coeff", "Decay exponential coefficiant", {.floatv = 0.0}},
   /*{DSOPTION_FLOAT, "attack_s", "Attack ending in samples", {.intv = 0}},
   {DSOPTION_FLOAT, "decay_s", "Decay begin in samples", {.intv=  0}},*/
   {.option_name = NULL}
};
/* clang-format on */

static void reset_options_to_default(void) {
  for (int i = 0; options[i].option_name != NULL; i++) {
    switch (options[i].type) {
    case DSOPTION_FLOAT:
      options[i].value.floatv = 0.0;
      continue;
    case DSOPTION_STRING:
      options[i].value.strv = NULL;
      continue;
    }
  }
}

static double linear_attack(double x, double n) { return x / n; }

static double linear_release(double x, double n) { return x / n; }

static double exponential_attack(double x, double n) { return pow(M_E, x - n); }
static double exponential_release(double x, double n) {
  return pow(M_E, (-x - n) + 1.0);
}

double (*attack_ret)(double, double) = NULL;
double (*release_ret)(double, double) = NULL;

static void write_note(int16_t *pcm, void *args, note_t *note,
                       uint32_t seek_pcm) {
  for (uint32_t i = 0; i < note->duration; i++) {
    double sample1 =
        ((double)note->volume *
         sin(2.0 * M_PI *
             (note->frequency +
              sin(2.0 * M_PI * options[0].value.floatv * (double)i / 44100.0) *
                  options[1].value.floatv) *
             (double)i / 44100.0));

    pcm[seek_pcm + i] +=
        (int16_t)
        /* simple linear attack and linear decay filter */
        (double)sample1 *
        (i < (uint32_t)note->attack
             ? attack_ret((double)i, note->attack)
             : (i > note->duration - (uint32_t)note->release
                    ? release_ret(-(double)i + note->duration, note->release)
                    : 1.0));
  }
}

static int synth(const symrec_t *const staff, track_t *const pcm_ctx) {
  if (options[2].value.floatv == 0.0)
    attack_ret = linear_attack;
  else
    attack_ret = exponential_attack;

  if (options[5].value.floatv == 0.0)
    release_ret = linear_release;
  else
    release_ret = exponential_release;

  uint32_t tnb_samples = staff->value.staff.nb_samples + 1024;

  int16_t *pcm, *lpcm, *rpcm;
  switch (pcm_ctx->track_type) {
  case 0:
    pcm = calloc(tnb_samples, sizeof(int16_t));
    if (pcm == NULL) {
      DSYNTH_LOG("no mem\n");
      return 1;
    }
    write_block(pcm, NULL, staff->value.staff.bnr, write_note);
    break;
  case 1:
    lpcm = calloc(tnb_samples, sizeof(int16_t));
    rpcm = calloc(tnb_samples, sizeof(int16_t));
    if (lpcm == NULL || rpcm == NULL) {
      DSYNTH_LOG("no mem\n");
      return 1;
    }
    write_block(lpcm, NULL, staff->value.staff.bnr, write_note);
    write_block(rpcm, NULL, staff->value.staff.bnr, write_note);
    break;
  }

  for (DSOption *ctx = options; ctx->option_name != NULL; ctx++) {
    printf("[s_sin] %s ", ctx->option_name);
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
  switch (pcm_ctx->track_type) {
  case 0:
    pcm_ctx->mono.pcm = pcm;
    pcm_ctx->mono.nb_samples = tnb_samples;
    pcm_ctx->mono.play_end = staff->value.staff.nb_samples;
    break;
  case 1:
    pcm_ctx->stereo.lpcm = lpcm;
    pcm_ctx->stereo.rpcm = rpcm;
    pcm_ctx->stereo.lnb_samples = tnb_samples;
    pcm_ctx->stereo.rnb_samples = tnb_samples;
    pcm_ctx->stereo.lplay_end = staff->value.staff.nb_samples;
    pcm_ctx->stereo.rplay_end = staff->value.staff.nb_samples;
    break;
  }
  reset_options_to_default();
  return 0;
}

/* clang-format off */
DSSynth ss_sin = {
  .name = "sin",
  .description = "A sine wave synth",
  .options = options,
  .synth = &synth
};
/* clang-format on */
