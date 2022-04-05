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

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include "env.h"
#include "libdfilter/allfilter.h"
#include "libdsynth/allsynth.h"

extern symrec_t *getsym(dats_t *, char *);

int gen_pcm16(dats_t *dats, pcm16_t *ctx) {
  if (ctx->pcm !=  NULL){
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
    int (*const synth_func)(const symrec_t *const, pcm16_t *const) = synth_ctx->synth;
    if (synth_func(getsym(dats, ctx->SYNTH.staff_name), ctx)){
      printf("ERROR -->\n");
      return 1;

    }
   break;/*
   case FILTER:
   synth_ctx = get_dfult */

  default:
    return 1;
  }

  return 0;
}
