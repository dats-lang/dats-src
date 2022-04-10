#include "wav.h"
#include <assert.h>
#include <env.h>
#include <stdint.h>
#include <stdlib.h>

extern int gen_pcm16(dats_t *, pcm16_t *);
extern int16_t mix16(int16_t, int16_t, float);

int exec_write(dats_t *dats) {
  for (symrec_t *n = dats->sym_table; n != NULL; n = n->next) {
    if (n->type != TOK_WRITE)
      continue;
    FILE *fp = fopen(n->value.write.out_file, "wb");
    if (fp == NULL) {
      perror(n->value.write.out_file);
      return 1;
    }
    for (pcm16_t *pcm16 = n->value.write.pcm; pcm16 != NULL;
         pcm16 = pcm16->next)
      gen_pcm16(dats, pcm16);
    uint32_t tnb_samples = 0;
    for (pcm16_t *pcm16 = n->value.write.pcm; pcm16 != NULL;
         pcm16 = pcm16->next) {
      if (pcm16->next != NULL) {
        tnb_samples += pcm16->play_end;
      } else
        tnb_samples += pcm16->nb_samples;
    }

    struct WAV_info wav = {
        .fp = fp,
        .Subchunk1Size = 16,
        .AudioFormat = 1,
        .NumChannels = 1,
        .SampleRate = 44100,
        .NumSamples = tnb_samples,
        .BitsPerSample = 16,
    };
    wav_write_header(&wav);
    int16_t *out_pcm = calloc(tnb_samples, sizeof(int16_t));
    assert(out_pcm != NULL);
    uint32_t seek_pcm = 0;
    for (pcm16_t *pcm16 = n->value.write.pcm; pcm16 != NULL;
         pcm16 = pcm16->next) {
      for (uint32_t n = 0; n < pcm16->nb_samples; n++)
        out_pcm[seek_pcm + n] =
            mix16(out_pcm[seek_pcm + n], pcm16->pcm[n], pcm16->gain);
      // out_pcm[seek_pcm + n] += (int16_t)((float)pcm16->pcm[n]*pcm16->gain);
      seek_pcm += pcm16->play_end;
    }
    fwrite(out_pcm, sizeof(int16_t), tnb_samples, fp);
    free(out_pcm);
    fclose(fp);
  }
  return 0;
}
