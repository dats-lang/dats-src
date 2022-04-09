#include "wav.h"
#include <env.h>
#include <stdint.h>

extern int gen_pcm16(dats_t *, pcm16_t *);

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
    for (pcm16_t *pcm16 = n->value.write.pcm; pcm16 != NULL;
         pcm16 = pcm16->next) {
      if (pcm16->next != NULL)
        fwrite(pcm16->pcm, sizeof(int16_t), pcm16->play_end, fp);
      else
        fwrite(pcm16->pcm, sizeof(int16_t), pcm16->nb_samples, fp);
    }
    fclose(fp);
  }
  return 0;
}
