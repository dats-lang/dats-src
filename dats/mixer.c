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

void memmix16(int16_t *dst, int16_t *src, float gain, uint32_t nb_samples) {
  for (uint32_t n = 0; n < nb_samples; n++) {
    // dst[n] += (int16_t)((float)src[n] * pow(gain, 2.0f));
    dst[n] += (int16_t)((float)src[n] * pow(M_E, (gain - 1.0) * 4.0f));
  }
}

int16_t mix16(int16_t a, int16_t b, float gain) {
  /* This might be slow */
  return a + (int16_t)((float)b * pow(M_E, (gain - 1.0) * 4.0f));
}
