#include "sndfilter/biquad.h"
#include "sndfilter/compressor.h"
#include "sndfilter/reverb.h"
#include "sndfilter/wav.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "filter.h"
#include "log.h"

/* clang-format off */
static DFOption options[] = {
   /*{DFOPTION_INT, "attack_s", "Attack ending in samples", {.intv = 0}},
   {DFOPTION_INT, "decay_s", "Decay begin in samples", {.intv=  0}},*/
   {.option_name = NULL}
};
/* clang-format on */

static void free_string_options(void) {
  for (int i = 0; options[i].option_name != NULL; i++) {
    if (options[i].type != DFOPTION_STRING) {
      options[i].value.intv = 0;
      options[i].value.floatv = 0.0;
      continue;
    }
    free(options[i].value.strv);
  }
}

static int filter(track_t *pcm_ctx, track_t *const pcm_src) {

  // read the data and convert to stereo floating point
  int16_t L, R;
  sf_snd snd;
  uint32_t nb_samples = 0;
  switch (pcm_src->track_type) {
  case 0:
    nb_samples = pcm_src->mono.nb_samples;
    snd = sf_snd_new(nb_samples, 44100, false);
#pragma omp parallel for
    for (uint32_t i = 0; i < nb_samples; i++) {
      // read the sample
      L = pcm_src->mono.pcm[i];
      R = L; // expand to stereo

      // convert the sample to floating point
      // notice that int16 samples range from -32768 to 32767, therefore we have
      // a different divisor depending on whether the value is negative or not

      if (L < 0)
        snd->samples[i].L = (float)L / 32768.0f;
      else
        snd->samples[i].L = (float)L / 32767.0f;
      if (R < 0)
        snd->samples[i].R = (float)R / 32768.0f;
      else
        snd->samples[i].R = (float)R / 32767.0f;
    }
    break;
  case 1:
    nb_samples = (pcm_src->stereo.lnb_samples > pcm_src->stereo.rnb_samples)
                     ? pcm_src->stereo.lnb_samples
                     : pcm_src->stereo.rnb_samples;
    snd = sf_snd_new(nb_samples, 44100, false);
    for (uint32_t i = 0; i < nb_samples; i++) {
      // read the sample
      L = pcm_src->stereo.lpcm[i];
      R = pcm_src->stereo.rpcm[i];

      // convert the sample to floating point
      // notice that int16 samples range from -32768 to 32767, therefore we have
      // a different divisor depending on whether the value is negative or not

      if (L < 0)
        snd->samples[i].L = (float)L / 32768.0f;
      else
        snd->samples[i].L = (float)L / 32767.0f;
      if (R < 0)
        snd->samples[i].R = (float)R / 32768.0f;
      else
        snd->samples[i].R = (float)R / 32767.0f;
    }
    break;
  }
  sf_reverb_preset p = SF_REVERB_PRESET_DEFAULT;
  sf_snd output_snd = sf_snd_new(nb_samples + 44100, 44100, true);
  if (output_snd == NULL) {
    DFILTER_LOG("Error: Failed to apply filter\n");
    return 1;
  }

  // process the reverb in one sweep
  sf_reverb_state_st rv;
  sf_presetreverb(&rv, 44100, p);
  sf_reverb_process(&rv, nb_samples, snd->samples, output_snd->samples);

  // append the tail
  int tailsmp = 44100;
  if (tailsmp > 0) {
    int pos = nb_samples;
    sf_sample_st *empty = malloc(sizeof(sf_sample_st) * 48000);
    memset(empty, 0, sizeof(sf_sample_st) * 48000);
    while (tailsmp > 0) {
      if (tailsmp <= 48000) {
        sf_reverb_process(&rv, tailsmp, empty, &output_snd->samples[pos]);
        break;
      } else {
        sf_reverb_process(&rv, 48000, empty, &output_snd->samples[pos]);
        tailsmp -= 48000;
        pos += 48000;
      }
    }
    free(empty);
  }

  putchar('\n');
  for (DFOption *ctx = options; ctx->option_name != NULL; ctx++) {
    printf("[f_reverb] %s ", ctx->option_name);
    switch (ctx->type) {
    case DFOPTION_FLOAT:
      printf("%f", ctx->value.floatv);
      break;
    case DFOPTION_INT:
      printf("%d", ctx->value.intv);
      break;
    case DFOPTION_STRING:
      printf("%s", ctx->value.strv != NULL ? ctx->value.strv : " ");
      break;
    }
    putchar('\n');
  }
  switch (pcm_ctx->track_type) {
  case 0:
    pcm_ctx->mono.nb_samples = output_snd->size;
    pcm_ctx->mono.pcm = malloc(sizeof(int16_t) * output_snd->size);
#pragma omp parallel for
    for (int i = 0; i < output_snd->size; i++) {
      float R = output_snd->samples[i].R;
      if (R < 0)
        pcm_ctx->mono.pcm[i] = (int16_t)(R * 32768.0f);
      else
        pcm_ctx->mono.pcm[i] = (int16_t)(R * 32767.0f);
    }
    break;
  case 1:
    pcm_ctx->stereo.lnb_samples = output_snd->size;
    pcm_ctx->stereo.rnb_samples = output_snd->size;
    pcm_ctx->stereo.lpcm = malloc(sizeof(int16_t) * output_snd->size);
    pcm_ctx->stereo.rpcm = malloc(sizeof(int16_t) * output_snd->size);
    assert(pcm_ctx->stereo.lpcm != NULL && pcm_ctx->stereo.rpcm != NULL);
    for (int i = 0; i < output_snd->size; i++) {
      pcm_ctx->stereo.lpcm[i] = (int16_t)(output_snd->samples[i].L * 32768.0f);
      pcm_ctx->stereo.rpcm[i] = (int16_t)(output_snd->samples[i].R * 32767.0f);
    }
    break;
  }

  free(output_snd->samples);
  free(output_snd);
  free_string_options();
  return 0;
}

/* clang-format off */
DFFilter f_reverb = {
  .name = "reverb",
  .description = "A reverb filter",
  .options = options,
  .filter = &filter
};
/* clang-format on */
