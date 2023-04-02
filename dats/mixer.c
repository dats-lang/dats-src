/*  Dats interpreter
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

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

void memmixf(float *dst, float *src, float gain, uint32_t nb_samples) {

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
  uint32_t remaining = (nb_samples % 8);
  uint32_t n = 0, trunc = nb_samples - remaining;
  for (; n < truncs; n+=8) {
    // dst[n] += (int16_t)((float)src[n] * pow(gain, 2.0f));
    int16x8_t av = vld1q_s16(a+n);
    int16x8_t bv = vld1q_s16(a+n);
  
    int16x8_t targetv = vmlaq_s16(av, bv);
  
    /* Store the result */
    vst1q_s16(dst + n, targetv);
  }
#else
  for (uint32_t n = 0; n < nb_samples; n++) {
    // dst[n] += (int16_t)((float)src[n] * pow(gain, 2.0f));
    dst[n] += (int16_t)((float)src[n] * pow(M_E, (gain - 1.0) * 4.0f));
  }
#endif
}

void memmixs16(int16_t *dst, int16_t *src, float gain, uint32_t nb_samples) {

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
  uint32_t remaining = (nb_samples % 8);
  uint32_t n = 0, trunc = nb_samples - remaining;
  for (; n < truncs; n+=8) {
    // dst[n] += (int16_t)((float)src[n] * pow(gain, 2.0f));
    int16x8_t av = vld1q_s16(a+n);
    int16x8_t bv = vld1q_s16(a+n);
  
    int16x8_t targetv = vmlaq_s16(av, bv);
  
    /* Store the result */
    vst1q_s16(dst + n, targetv);
  }
#else
  for (uint32_t n = 0; n < nb_samples; n++) {
    // dst[n] += (int16_t)((float)src[n] * pow(gain, 2.0f));
    dst[n] += (int16_t)((float)src[n] * pow(M_E, (gain - 1.0) * 4.0f));
  }
#endif
}

int16_t mixs16(int16_t a, int16_t b, float gain) {
  /* This might be slow */
  return a + (int16_t)((float)b * pow(M_E, (gain - 1.0) * 4.0f));
}
