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

#include "env.h"
#include "libdfilter/allfilter.h"
#include "libdsynth/allsynth.h"
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

extern symrec_t *getsym(dats_t *, char *);
extern void memmix16(int16_t *, int16_t *, float, uint32_t);

int gen_pcm16(dats_t *dats, pcm16_t *ctx) {
  if (ctx->pcm != NULL) {
    /* We're done here */
    return 0;
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
    int (*const synth_func)(const symrec_t *const, pcm16_t *const) =
        synth_ctx->synth;
    if (synth_func(getsym(dats, ctx->SYNTH.staff_name), ctx)) {
      printf("ERROR -->\n");
      return 1;
    }
    break;
  case FILTER:
    filter_ctx = get_dfilter_by_name(ctx->FILTER.filter_name);
    int (*const filter_func)(pcm16_t * dst, pcm16_t * src) = filter_ctx->filter;
    for (pcm16_t *pcm16_arg = ctx->FILTER.pcm16_arg; pcm16_arg != NULL;
         pcm16_arg = pcm16_arg->next) {
      if (pcm16_arg->pcm == NULL)
        gen_pcm16(dats, pcm16_arg);
    }
    if (filter_func(ctx, ctx->FILTER.pcm16_arg))
      return 1;
    break;
  case ID: {
    pcm16_t *src = getsym(dats, ctx->ID.id)->value.pcm16.pcm;
    if (src->pcm == NULL)
      gen_pcm16(dats, src);
    ctx->pcm = malloc(src->nb_samples * sizeof(int16_t));
    assert(ctx->pcm != NULL);
    memcpy(ctx->pcm, src->pcm, src->nb_samples * sizeof(int16_t));
    ctx->nb_samples = src->nb_samples;
    ctx->play_end = src->play_end;
    ctx->gain = src->gain;
  } break;
  case MIX:
    for (uint32_t i = 0; i < ctx->MIX.nb_pcm16; i++)
      for (pcm16_t *src = ctx->MIX.pcm16[i]; src != NULL; src = src->next)
        if (src->pcm == NULL)
          gen_pcm16(dats, src);

    {
      uint32_t anb_samples = 0, bnb_samples = 0;
      for (uint32_t i = 0; i < ctx->MIX.nb_pcm16; i++) {
        for (pcm16_t *src = ctx->MIX.pcm16[i]; src != NULL; src = src->next)
          anb_samples += src->nb_samples;
        if (anb_samples > bnb_samples)
          bnb_samples = anb_samples;
        anb_samples = 0;
      }
      ctx->pcm = calloc(bnb_samples, sizeof(int16_t));
      ctx->nb_samples = bnb_samples;
      assert(ctx->pcm != NULL);
    }
    for (uint32_t i = 0; i < ctx->MIX.nb_pcm16; i++) {
      uint32_t look = 0;
      for (pcm16_t *src = ctx->MIX.pcm16[i]; src != NULL; src = src->next) {
        memmix16(ctx->pcm + look, src->pcm, src->gain, src->nb_samples);
        look += src->play_end;
        if (i == ctx->MIX.nb_pcm16 - 1 && src->next == NULL)
          ctx->play_end = src->play_end;
      }
    }
    break;
  default:
    return 1;
  }

  return 0;
}
