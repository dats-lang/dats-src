/*  pcm16_s16le generator
 *
 * Copyright (c) 2022 Al-buharie Amjari
 *
 * This file is part of Dats.
 *
 * Dats is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Dats is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Dats; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "dats.h"
#include "env.h"
#include "libdfilter/allfilter.h"
#include "libdsynth/allsynth.h"
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

extern symrec_t *getsym(dats_t *, char *);
extern void memmix16(int16_t *, int16_t *, float, uint32_t);

int gen_pcm16(dats_t *dats, track_t *ctx) {
  switch (ctx->track_type) {
  case 0:
    if (ctx->mono.pcm != NULL) {
      /* We're done here */
      return 0;
    }
    break;
  case 1:
    if (ctx->stereo.lpcm != NULL && ctx->stereo.rpcm != NULL) {
      /* We're done here */
      return 0;
    }
    break;
  }

  DSSynth *synth_ctx = NULL;
  DFFilter *filter_ctx = NULL;
  if (ctx == NULL)
    return 1;
  switch (ctx->type) {
  case SYNTH:
    synth_ctx = get_dsynth_by_name(ctx->SYNTH.synth_name);
    for (int op = 0; op < ctx->SYNTH.nb_options; op++) {
      int ctr = 0;
      while (synth_ctx->options[ctr].option_name != NULL) {
        if (!strcmp(ctx->SYNTH.options[op].option_name,
                    synth_ctx->options[ctr].option_name)) {
          printf("option found %s %d\n", ctx->SYNTH.options[op].option_name,
                 op);
          switch (synth_ctx->options[ctr].type) {
          case DSOPTION_FLOAT:
            printf("float %f\n", ctx->SYNTH.options[op].floatv);
            synth_ctx->options[ctr].value.floatv =
                ctx->SYNTH.options[op].floatv;
            break;
          case DSOPTION_STRING:
            synth_ctx->options[ctr].value.strv = ctx->SYNTH.options[op].strv;
            break;
          }
          break;
        }
        ctr++;
      }
    }
    int (*const synth_func)(const symrec_t *const, track_t *const) =
        synth_ctx->synth;
    if (synth_func(getsym(dats, ctx->SYNTH.staff_name), ctx)) {
      printf("ERROR -->\n");
      return 1;
    }
    break;
  case FILTER:
    filter_ctx = get_dfilter_by_name(ctx->FILTER.filter_name);
    int (*const filter_func)(track_t * dst, track_t * src) = filter_ctx->filter;
    for (track_t *pcm16_arg = ctx->FILTER.pcm16_arg; pcm16_arg != NULL;
         pcm16_arg = pcm16_arg->next) {
      switch (pcm16_arg->track_type) {
      case 0:
        if (pcm16_arg->mono.pcm == NULL)
          gen_pcm16(dats, pcm16_arg);
        break;
      case 1:
        if (pcm16_arg->stereo.lpcm == NULL && pcm16_arg->stereo.rpcm == NULL)
          gen_pcm16(dats, pcm16_arg);
        break;
      }
    }
    if (filter_func(ctx, ctx->FILTER.pcm16_arg))
      return 1;
    break;
  case ID: {
    track_t *src = getsym(dats, ctx->ID.id)->value.pcm16.pcm;
    switch (src->track_type) {
    case 0:
      if (src->mono.pcm == NULL)
        gen_pcm16(dats, src);
      break;
    case 1:
      if (src->stereo.lpcm == NULL && src->stereo.rpcm == NULL)
        gen_pcm16(dats, src);
      break;
    }

    switch (ctx->track_type) {
    case 0:
      ctx->mono.pcm = malloc(src->mono.nb_samples * sizeof(int16_t));
      assert(ctx->mono.pcm != NULL);
      memcpy(ctx->mono.pcm, src->mono.pcm,
             src->mono.nb_samples * sizeof(int16_t));
      ctx->mono.nb_samples = src->mono.nb_samples;
      ctx->mono.play_end = src->mono.play_end;
      break;
    case 1:
      ctx->stereo.lpcm = malloc(src->stereo.lnb_samples * sizeof(int16_t));
      assert(ctx->stereo.lpcm != NULL);
      memcpy(ctx->stereo.lpcm, src->stereo.lpcm,
             src->stereo.lnb_samples * sizeof(int16_t));
      ctx->stereo.lnb_samples = src->stereo.lnb_samples;
      ctx->stereo.lplay_end = src->stereo.lplay_end;

      ctx->stereo.rpcm = malloc(src->stereo.rnb_samples * sizeof(int16_t));
      assert(ctx->stereo.rpcm != NULL);
      memcpy(ctx->stereo.rpcm, src->stereo.rpcm,
             src->stereo.rnb_samples * sizeof(int16_t));
      ctx->stereo.rnb_samples = src->stereo.rnb_samples;
      ctx->stereo.rplay_end = src->stereo.rplay_end;
      break;
    }
    ctx->gain = src->gain;
  } break;
  case MIX:
    for (uint32_t i = 0; i < ctx->MIX.nb_pcm16; i++)
      for (track_t *src = ctx->MIX.pcm16[i]; src != NULL; src = src->next) {
        switch (src->track_type) {
        case 0:
          if (src->mono.pcm == NULL)
            gen_pcm16(dats, src);
          break;
        case 1:
          if (src->stereo.lpcm == NULL)
            gen_pcm16(dats, src);
          break;
        }
      }

    //   {
    uint32_t anb_samples = 0, bnb_samples = 0;
    for (uint32_t i = 0; i < ctx->MIX.nb_pcm16; i++) {
      for (track_t *src = ctx->MIX.pcm16[i]; src != NULL; src = src->next) {
        switch (src->track_type) {
        case 0:
          anb_samples += src->mono.nb_samples;
          break;
        case 1:
          anb_samples += (src->stereo.lnb_samples > src->stereo.rnb_samples
                              ? src->stereo.lnb_samples
                              : src->stereo.rnb_samples);
          break;
        }
      }
      if (anb_samples > bnb_samples)
        bnb_samples = anb_samples;
      anb_samples = 0;
    }
    switch (ctx->track_type) {
    case 0:
      ctx->mono.pcm = calloc(bnb_samples, sizeof(int16_t));
      assert(ctx->mono.pcm != NULL);
      ctx->mono.nb_samples = bnb_samples;
      break;
    case 1:
      ctx->stereo.lpcm = malloc(bnb_samples * sizeof(int16_t));
      assert(ctx->stereo.lpcm != NULL);
      ctx->stereo.lnb_samples = bnb_samples;

      ctx->stereo.rpcm = calloc(bnb_samples, sizeof(int16_t));
      assert(ctx->stereo.rpcm != NULL);
      ctx->stereo.rnb_samples = bnb_samples;
      break;
    }

    uint32_t seek = 0, lseek = 0, rseek = 0;
    for (uint32_t i = 0; i < ctx->MIX.nb_pcm16; i++){
      for (track_t *src = ctx->MIX.pcm16[i]; src != NULL; src = src->next) {
        switch (ctx->track_type) {
        case 0:
          /* Is src also mono? */
          if (src->track_type == 1)
            DATS_LOG("warning, mixing mono track with stereo track\n");
          memmix16(ctx->mono.pcm + seek, src->mono.pcm, src->gain,
                   src->mono.nb_samples);
          seek += src->mono.play_end;
          if (i == ctx->MIX.nb_pcm16 - 1 && src->next == NULL)
            ctx->mono.play_end = src->mono.play_end;
          break;
        case 1:
          /* Is src also stereo? */
          if (src->track_type == 0)
            DATS_LOG("warning, mixing stereo track with mono track\n");
          memmix16(ctx->stereo.lpcm + lseek, src->stereo.lpcm, src->gain,
                   src->stereo.lnb_samples);
          lseek += src->stereo.lplay_end;
          if (i == ctx->MIX.nb_pcm16 - 1 && src->next == NULL)
            ctx->stereo.lplay_end = src->stereo.lplay_end;

          memmix16(ctx->stereo.rpcm + rseek, src->stereo.rpcm, src->gain,
                   src->stereo.rnb_samples);
          rseek += src->stereo.rplay_end;
          if (i == ctx->MIX.nb_pcm16 - 1 && src->next == NULL)
            ctx->stereo.rplay_end = src->stereo.rplay_end;
        }
      }
    seek = 0; lseek = 0; rseek = 0;
    }

    break;
  default:
    return 1;
  }

  return 0;
}
