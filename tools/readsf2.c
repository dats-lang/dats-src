/* sf2.c - sf2 lib for reading sf2 files
 *
 * Copyright (c) 2021 Al-buharie Amjari
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 2)
typedef struct phdr_t phdr_t;
struct phdr_t {
  char name[20];
  uint16_t preset;
  uint16_t bank;
  uint16_t pbag_index;
  uint32_t library;
  uint32_t genre;
  uint32_t morphology;
};

typedef struct pbag_t pbag_t;
struct pbag_t {
  uint16_t gen_index;
  uint16_t mod_index;
};

typedef struct pmod_t pmod_t;
struct pmod_t {
  uint16_t a;
  uint16_t b;
  uint16_t c;
  uint16_t d;
  uint16_t e;
};

typedef struct pgen_t pgen_t;
struct pgen_t {
  uint16_t a;
  uint16_t b;
};

typedef struct inst_t inst_t;
struct inst_t {
  char name[20];
  uint16_t ibag_index;
};

typedef struct ibag_t ibag_t;
struct ibag_t {
  uint16_t igen_index;
  uint16_t imod_index;
};

typedef struct imod_t imod_t;
struct imod_t {
  uint16_t a;
  uint16_t b;
  uint16_t c;
  uint16_t d;
  uint16_t e;
};

typedef struct igen_t igen_t;
struct igen_t {
  uint16_t a;
  uint16_t b;
};

typedef struct shdr_t shdr_t;
struct shdr_t {
  char name[20];
  uint32_t start;
  uint32_t end;
  uint32_t start_loop;
  uint32_t end_loop;
  uint32_t sample_rate;
  int8_t pitch_correction;
  uint16_t sample_link;
  uint16_t sample_type;
};
#pragma pack(pop)

typedef struct SF2 SF2;
struct SF2 {
  uint32_t nb_smpl;
  int16_t *smpl;

  uint32_t nb_phdr;
  phdr_t *phdr;

  uint32_t nb_pbag;
  pbag_t *pbag;

  uint32_t nb_pmod;
  pmod_t *pmod;

  uint32_t nb_pgen;
  pgen_t *pgen;

  uint32_t nb_inst;
  inst_t *inst;

  uint32_t nb_ibag;
  ibag_t *ibag;

  uint32_t nb_imod;
  imod_t *imod;

  uint32_t nb_igen;
  igen_t *igen;

  uint32_t nb_shdr;
  shdr_t *shdr;
};

#define STR4CMP(s1, s2) strncmp(s1, s2, 4)
#define CORRUPTED(errno)                                                       \
  {                                                                            \
    sf2_errno = errno;                                                         \
    fprintf(stderr, "%s:%d ", __FILE__, __LINE__);                             \
    sf2_perror(argv[1]);                                                       \
    sf2_destroy_sf2(sf2);                                                      \
    return 1;                                                                  \
  }

int sf2_errno = 0;
const char *const sf2_errlist[] = {
    "no error",
    "unrecognized file format, or maybe corrupted",
    "structurally unsound",
    "no memory",
    "unsupported version",
    "no such preset",
};

void sf2_perror(char *str) {
  fprintf(stderr, "%s: %s\n", str, sf2_errlist[sf2_errno]);
}

void sf2_destroy_sf2(SF2 *sf2) {
  if (sf2 == NULL)
    return;
  free(sf2->smpl);

  free(sf2->phdr);
  free(sf2->pbag);
  free(sf2->pmod);
  free(sf2->pgen);

  free(sf2->inst);
  free(sf2->ibag);
  free(sf2->imod);
  free(sf2->igen);

  free(sf2->shdr);

  free(sf2);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s file.sf2\n", argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "rb");
  if (fp == NULL) {
    perror(argv[1]);
    return 1;
  }
  SF2 *sf2 = NULL;

  char utag[4] = {0};
  uint32_t usize = 0;
  fread(utag, 1, 4, fp);
  if (STR4CMP(utag, "RIFF"))
    CORRUPTED(1);

  fseek(fp, 4, SEEK_CUR);
  fread(utag, 1, 4, fp);
  if (STR4CMP(utag, "sfbk"))
    CORRUPTED(1);

  fread(utag, 1, 4, fp);
  if (STR4CMP(utag, "LIST"))
    CORRUPTED(1);

  fread(&usize, 4, 1, fp);
  fread(utag, 1, 4, fp);
  if (strncmp(utag, "INFO", 4))
    CORRUPTED(1);
  printf("\nLIST INFO:\n");
  printf("  LIST-INFO size:                %d\n", usize);

  while (1) {
    if (fread(utag, 1, 4, fp) != 4)
      CORRUPTED(1);
    if (fread(&usize, sizeof(uint32_t), 1, fp) != 1)
      CORRUPTED(1);
    if (!strncmp(utag, "ifil", 4)) {

      if (usize != (uint32_t)4)
        CORRUPTED(1);

      uint16_t wMajor = 0;
      uint16_t wMinor = 0;
      if (fread(&wMajor, sizeof(uint16_t), 1, fp) != 1)
        CORRUPTED(1);
      if (fread(&wMinor, sizeof(uint16_t), 1, fp) != 1)
        CORRUPTED(1);

      printf("  Version:                       %hi.%-10hi\n", wMajor, wMinor);
      if (wMajor != 2 || wMinor != 1)
        CORRUPTED(4);
    } else if (!strncmp(utag, "isng", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char SoundEngine[257] = {0};
      if (fread(SoundEngine, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  Target sound engine:           %s\n", SoundEngine);
    } else if (!strncmp(utag, "INAM", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char SoundBankName[257] = {0};
      if (fread(SoundBankName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  Sound bank name:               %s\n", SoundBankName);
    } else if (!strncmp(utag, "irom", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char ROMName[257] = {0};
      if (fread(ROMName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  IROM:                          %s\n", ROMName);

    } else if (!strncmp(utag, "iver", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char IVERName[257] = {0};
      if (fread(IVERName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  IVER:                          %s\n", IVERName);

    } else if (!strncmp(utag, "ICRD", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char ICRDName[257] = {0};
      if (fread(ICRDName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  ICRD:                          %s\n", ICRDName);

    } else if (!strncmp(utag, "IENG", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char IENGName[257] = {0};
      if (fread(IENGName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  IENG:                          %s\n", IENGName);

    } else if (!strncmp(utag, "IPRD", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char IPRDName[257] = {0};
      if (fread(IPRDName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  IPRD:                          %s\n", IPRDName);

    } else if (!strncmp(utag, "ICOP", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char ICOPName[257] = {0};
      if (fread(ICOPName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  ICOP:                          %s\n", ICOPName);

    } else if (!strncmp(utag, "ICMT", 4)) {
      if (usize > 65537)
        CORRUPTED(1);

      char ICMTName[65538] = {0};
      if (fread(ICMTName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  ICMT:                          %s\n", ICMTName);

    } else if (!strncmp(utag, "ISFT", 4)) {
      if (usize > 256)
        CORRUPTED(1);

      char TOOLName[257] = {0};
      if (fread(TOOLName, 1, usize, fp) != usize)
        CORRUPTED(1);
      printf("  Tool created:                  %s\n", TOOLName);

    } else
      break;
  }

  if (strncmp(utag, "LIST", 4))
    CORRUPTED(1);

  fread(utag, 1, 4, fp);
  if (strncmp(utag, "sdta", 4))
    CORRUPTED(1)
  printf("\nLIST sdta:\n");
  printf("  LIST-sdta size:                %d\n", usize);
  fread(utag, 1, 4, fp);
  if (strncmp(utag, "smpl", 4))
    CORRUPTED(1);
  fread(&usize, sizeof(uint32_t), 1, fp);
  printf("  smpl size:                     %d\n", usize);
  sf2 = calloc(1, sizeof(SF2));
  if (sf2 == NULL)
    CORRUPTED(3);

  sf2->smpl = NULL;
  sf2->phdr = NULL;
  sf2->pbag = NULL;
  sf2->pmod = NULL;
  sf2->pgen = NULL;
  sf2->inst = NULL;
  sf2->ibag = NULL;
  sf2->imod = NULL;
  sf2->igen = NULL;
  sf2->shdr = NULL;

  sf2->smpl = malloc(usize);
  if (sf2->smpl == NULL)
    CORRUPTED(3);
  fread(sf2->smpl, sizeof(int16_t), usize / 2, fp);
  sf2->nb_smpl = usize / 2;

  fread(utag, 1, 4, fp);
  if (strncmp(utag, "LIST", 4))
    CORRUPTED(1);

  fread(&usize, 1, 4, fp);
  fread(utag, 1, 4, fp);
  if (strncmp(utag, "pdta", 4))
    CORRUPTED(1);
  printf("\nLIST pdta:\n");
  printf("  LIST-pdta size                 %d\n", usize);

  while (1) {
    if (fread(utag, 1, 4, fp) != 4) {
      if (sf2->smpl == NULL || sf2->phdr == NULL || sf2->pbag == NULL ||
          sf2->pmod == NULL || sf2->pgen == NULL || sf2->inst == NULL ||
          sf2->ibag == NULL || sf2->imod == NULL || sf2->igen == NULL ||
          sf2->shdr == NULL)
        CORRUPTED(1);
      break;
    }
    if (fread(&usize, sizeof(uint32_t), 1, fp) != 1)
      CORRUPTED(1);
    if (!STR4CMP(utag, "phdr")) {
      printf("  phdr size                      %d\n", usize);
      if (usize % 38)
        CORRUPTED(2);
      phdr_t *presets = malloc(sizeof(phdr_t) * (usize / 38));
      assert(presets != NULL);
      if (fread(presets, sizeof(phdr_t), usize / 38, fp) != usize / 38) {
        CORRUPTED(1);
      }

      sf2->phdr = presets;
      sf2->nb_phdr = usize / 38;
      printf("    Presets:\n");
      for (uint32_t i = 0; i < usize / 38; i++) {
        printf("     %-3d %-19s   %-3d  %-3d  %-3d  %-3d  %-3d %-3d\n", i,
               presets[i].name, presets[i].preset, presets[i].bank,
               presets[i].pbag_index, presets[i].library, presets[i].genre,
               presets[i].morphology);
      }

    } else if (!STR4CMP(utag, "pbag")) {

      printf("  pbag size                      %d\n", usize);
      if (usize % 4)
        CORRUPTED(2);
      pbag_t *preset_bags = malloc(sizeof(pbag_t) * (usize / 4));
      assert(preset_bags != NULL);
      if (fread(preset_bags, sizeof(pbag_t), usize / 4, fp) != usize / 4) {
        CORRUPTED(1);
      }
      sf2->pbag = preset_bags;
      sf2->nb_pbag = usize / 4;
      printf("    Preset bags:\n");
      for (uint32_t i = 0; i < usize / 4; i++) {
        printf("     %-3d                   %-3d  %-3d\n", i,
               preset_bags[i].gen_index, preset_bags[i].mod_index);
      }
    } else if (!STR4CMP(utag, "pmod")) {
      printf("  pmod size                      %d\n", usize);
      if (usize % 10)
        CORRUPTED(2);
      pmod_t *mod_list = malloc(sizeof(pmod_t) * (usize / 10));
      assert(mod_list != NULL);
      if (fread(mod_list, sizeof(pmod_t), usize / 10, fp) != usize / 10) {
        CORRUPTED(1);
      }
      sf2->pmod = mod_list;
      sf2->nb_pmod = usize / 10;
      printf("    PModulator lists:\n");
      for (uint32_t i = 0; i < usize / 10; i++) {
        printf("     %-3d                   %-3d  %-3d  %-3d  %-3d"
               "  %-3d\n ",
               i, mod_list[i].a, mod_list[i].b, mod_list[i].c, mod_list[i].d,
               mod_list[i].e);
      }
    } else if (!STR4CMP(utag, "pgen")) {
      printf("  pgen size                      %d\n", usize);
      if (usize % 4)
        CORRUPTED(2);
      pgen_t *gen_list = malloc(sizeof(pgen_t) * (usize / 4));
      assert(gen_list != NULL);
      if (fread(gen_list, sizeof(pgen_t), usize / 4, fp) != usize / 4) {
        CORRUPTED(1);
      }
      sf2->pgen = gen_list;
      sf2->nb_pgen = usize / 4;
      printf("    PGenerator lists:\n");
      for (uint32_t i = 0; i < usize / 4; i++) {
        printf("     %-3d                   %-3d  %-3d\n", i, gen_list[i].a,
               gen_list[i].b);
      }
    } else if (!STR4CMP(utag, "inst")) {
      printf("  inst size                      %d\n", usize);
      if (usize % 22)
        CORRUPTED(2);
      inst_t *inst_list = malloc(sizeof(inst_t) * (usize / 22));
      assert(inst_list != NULL);
      if (fread(inst_list, sizeof(inst_t), usize / 22, fp) != usize / 22) {
        CORRUPTED(1);
      }
      sf2->inst = inst_list;
      sf2->nb_inst = usize / 22;
      printf("    Instrument lists:\n");
      for (uint32_t i = 0; i < usize / 22; i++) {
        printf("     %-3d %-19s   %-3d\n", i, inst_list[i].name,
               inst_list[i].ibag_index);
      }
    } else if (!STR4CMP(utag, "ibag")) {
      printf("  ibag size                      %d\n", usize);
      ibag_t *inst_bags = malloc(sizeof(ibag_t) * (usize / 4));
      assert(inst_bags != NULL);
      if (fread(inst_bags, sizeof(ibag_t), usize / 4, fp) != usize / 4) {
        CORRUPTED(1);
      }
      sf2->ibag = inst_bags;
      sf2->nb_ibag = usize / 4;
      printf("    Instrument bags:\n");
      for (uint32_t i = 0; i < usize / 4; i++) {
        printf("     %-3d                   %-3d  %-3d\n", i,
               inst_bags[i].igen_index, inst_bags[i].imod_index);
      }
    } else if (!STR4CMP(utag, "imod")) {
      printf("  imod size                      %d\n", usize);
      if (usize % 10)
        CORRUPTED(2);
      imod_t *mod_list = malloc(sizeof(imod_t) * (usize / 10));
      assert(mod_list != NULL);
      if (fread(mod_list, sizeof(imod_t), usize / 10, fp) != usize / 10) {
        CORRUPTED(1);
      }
      sf2->imod = mod_list;
      sf2->nb_imod = usize / 10;
      printf("    IModulator lists:\n");
      for (uint32_t i = 0; i < usize / 10; i++) {
        printf("     %-3d                   %-3d  %-3d  %-3d  %-3d"
               "  %-3d\n ",
               i, mod_list[i].a, mod_list[i].b, mod_list[i].c, mod_list[i].d,
               mod_list[i].e);
      }
    } else if (!STR4CMP(utag, "igen")) {
      printf("  igen size                      %d\n", usize);
      if (usize % 4)
        CORRUPTED(2);
      igen_t *gen_list = malloc(sizeof(igen_t) * (usize / 4));
      assert(gen_list != NULL);
      if (fread(gen_list, sizeof(igen_t), usize / 4, fp) != usize / 4) {
        CORRUPTED(1);
      }
      sf2->igen = gen_list;
      sf2->nb_igen = usize / 4;
      printf("    IGenerator lists:\n");
      for (uint32_t i = 0; i < usize / 4; i++) {
        printf("     %-3d                   %-3d  %-3d\n", i, gen_list[i].a,
               gen_list[i].b);
      }
    } else if (!STR4CMP(utag, "shdr")) {
      printf("  shdr size                      %d\n", usize);
      if (usize % 46)
        CORRUPTED(2);
      shdr_t *shdr_list = malloc(sizeof(shdr_t) * (usize / 46));
      assert(shdr_list != NULL);
      if (fread(shdr_list, sizeof(shdr_t), usize / 46, fp) != usize / 46) {
        CORRUPTED(1);
      }
      sf2->shdr = shdr_list;
      sf2->nb_shdr = usize / 46;
      printf("    Sample header lists:\n");
      for (uint32_t i = 0; i < usize / 46; i++) {
        printf("     %-5d                   %-20s  %-7d  %-7d  %-7d  %-7d  "
               "%-5d  %-5d\n",
               i, shdr_list[i].name, shdr_list[i].start, shdr_list[i].end,
               shdr_list[i].start_loop, shdr_list[i].end_loop,
               shdr_list[i].sample_rate, shdr_list[i].pitch_correction);
      }
    } else
      CORRUPTED(1);
  }
  sf2_destroy_sf2(sf2);
  return 0;
}
