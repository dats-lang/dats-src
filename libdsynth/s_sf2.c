#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define WITH_FLUIDSYNTH
#ifdef WITH_FLUIDSYNTH
#include <fluidsynth.h>
#else
#include "sf2.h"
#endif

#include "log.h"
#include "synth.h"
#include "utils.h"

/* clang-format off */
static DSOption options[] = {
   {DSOPTION_STRING, "sf2", "sf2 file", {.strv = NULL}},
   {DSOPTION_FLOAT, "preset", "preset num (default: 0)", {.floatv = 0}},
   {DSOPTION_FLOAT, "bank", "bank num (default: 0)", {.floatv = 0}},
   {.option_name = NULL}
};
/* clang-format on */

static void reset_options_to_default(void) {
  for (int i = 0; options[i].option_name != NULL; i++) {
    if (options[i].type != DSOPTION_STRING) {
      options[i].value.floatv = 0;
      options[i].value.floatv = 0.0;
      continue;
    }
    options[i].value.strv = NULL;
  }
}

void write_note(int16_t *pcm, void *args, note_t *note, uint32_t seek_pcm) {
  /* Play a note */
  fluid_synth_noteon(args, 0, note->mnkey, 60);
  fluid_synth_write_s16(args, note->duration, pcm, seek_pcm, 1, pcm, seek_pcm,
                        1);
  fluid_synth_noteoff(args, 0, note->mnkey);
}

static int synth(const symrec_t *const staff, track_t *const pcm_ctx) {
  char *sf2_name;
  if (options[0].value.strv == NULL) {
    fprintf(stderr, "[s_sf2] no sf2 file entered.");
    sf2_name =
#ifdef __TERMUX__
        __TERMUX_PREFIX__ "/share"
#elif defined(__unix__)
        "/usr/share"
#endif
                          "/soundfonts/default.sf2";
    printf(" using %s", sf2_name);
    fflush(stdout);

  } else
    sf2_name = options[0].value.strv;

  fluid_settings_t *settings;
  fluid_synth_t *synth = NULL;
  int err = 0;
  /* Create the settings object. This example uses the default
   * values for the settings. */
  settings = new_fluid_settings();
  if (settings == NULL) {
    DSYNTH_LOG("Failed to create the settings\n");
    err = 2;
    if (synth) {
      delete_fluid_synth(synth);
    }
    if (settings) {
      delete_fluid_settings(settings);
    }
    return 1;
  }
  /* Create the synthesizer */
  synth = new_fluid_synth(settings);
  if (synth == NULL) {
    DSYNTH_LOG("Failed to create the synthesizer\n");
    err = 3;
    if (synth) {
      delete_fluid_synth(synth);
    }
    if (settings) {
      delete_fluid_settings(settings);
    }
    return 1;
  }
  /* Load the soundfont */
  if (fluid_synth_sfload(synth, sf2_name, 1) == -1) {
    DSYNTH_LOG("Failed to load the SoundFont\n");
    err = 4;
    if (synth) {
      delete_fluid_synth(synth);
    }
    if (settings) {
      delete_fluid_settings(settings);
    }
    return 1;
  }
  fluid_synth_set_gain(synth, 5.0);
  int sfont_id, bank_num, preset_num;
  fluid_synth_get_program(synth, 0, &sfont_id, &bank_num, &preset_num);
  fluid_synth_program_select(synth, 0, sfont_id, (int)options[2].value.floatv,
                             (int)options[1].value.floatv);

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

  if (synth) {
    delete_fluid_synth(synth);
  }
  if (settings) {
    delete_fluid_settings(settings);
  }
  /*
    FILE *fp = fopen(sf2_name, "rb");
    if (fp == NULL) {
      fprintf(stderr, "[s_sf2] ");
      perror(sf2_name);
      return NULL;
    }

    SF2 *sf2_ctx = sf2_read_sf2(fp);
    fclose(fp);
    if (sf2_ctx == NULL) {
      fprintf(stderr, "[s_sf2] ");
      sf2_perror(sf2_name);
      return NULL;
    }

    printf("name: %s rate: %d start: %d end: %d\n", sf2_ctx->shdr[0].name,
           sf2_ctx->shdr[0].sample_rate, sf2_ctx->shdr[0].start,
           sf2_ctx->shdr[0].end);

     for (nr_t *n = staff->value.staff.nr; n != NULL; n = n->next) {
       if (n->type == SYM_NOTE) {
         for (note_t *nn = n->note; nn != NULL; nn = nn->next) {
           (sf2_generate_pcm(&pcm[total], nn->duration, options[1].value.strv,
    sf2_ctx) ? sf2_perror("183") : 0);
         }
       }
       total += n->length;
       if ((total % 44100) < 1000) {
         printf("\r[s_sf2] %d/%d", total, staff->value.staff.nb_samples);
         fflush(stdout);
       }
     }*/
  for (DSOption *ctx = options; ctx->option_name != NULL; ctx++) {
    printf("[s_sf2] %s ", ctx->option_name);
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

  //  sf2_destroy_sf2(sf2_ctx);
  return 0;
}

DSSynth ss_sf2 = {
    .name = "sf2",
    .description = "A sf2 synth",
    .options = options,
    .synth = &synth,
};
