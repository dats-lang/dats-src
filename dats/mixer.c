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

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

static float reduction(float k1, float k2){
  return 1.0;
 // return (k1+k2)*pow(10.0, (fabsf(k1+k2)/3.0)-1.0);
}

void mix16(int16_t *dst, int16_t *src, uint32_t nb_samples) {
 for (uint32_t n = 0; n < nb_samples; n++){
   /* This might be slow */
   float k1 = (float)src[n]/(float)INT16_MAX;
   float k2 = (float)dst[n]/(float)INT16_MAX;
   dst[n] = (int16_t)(reduction(k1, k2)*(k1+k2)*(float)INT16_MAX);
 }
}
