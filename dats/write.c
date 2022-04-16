#include "wav.h"
#include <assert.h>
#include <env.h>
#include <stdint.h>
#include <stdlib.h>

extern int gen_pcm16(dats_t *, track_t *);
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
    for (track_t *pcm16 = n->value.write.pcm; pcm16 != NULL;
         pcm16 = pcm16->next)
      gen_pcm16(dats, pcm16);
    uint32_t tnb_samples = 0;
    for (track_t *pcm16 = n->value.write.pcm; pcm16 != NULL;
         pcm16 = pcm16->next) {
      switch (pcm16->track_type) {
      case 0:
          tnb_samples += pcm16->mono.nb_samples; break;
      case 1:
        if (pcm16->next != NULL) {
          tnb_samples += pcm16->stereo.lplay_end;
        } else
          tnb_samples += pcm16->stereo.lnb_samples;

        if (pcm16->next != NULL) {
          tnb_samples += pcm16->stereo.rplay_end;
        } else
          tnb_samples += pcm16->stereo.rnb_samples;
        break;
      }

      struct WAV_info wav = {
          .fp = fp,
          .Subchunk1Size = 16,
          .AudioFormat = 1,
          .NumChannels = (n->value.write.pcm->track_type) ? 2 : 1,
          .SampleRate = 44100,
          .NumSamples = /*(n->value.write.pcm->track_type ? 2 : 1)*/tnb_samples,
          .BitsPerSample = 16,
      };
      wav_write_header(&wav);
      int16_t *out_pcm = calloc(tnb_samples * wav.NumChannels, sizeof(int16_t));
      assert(out_pcm != NULL);

      switch (n->value.write.pcm->track_type) {
      case 0: {
        uint32_t seek_pcm = 0;
        for (track_t *pcm16 = n->value.write.pcm; pcm16 != NULL;
             pcm16 = pcm16->next) {
          for (uint32_t n = 0; n < pcm16->mono.nb_samples; n++)
            out_pcm[seek_pcm + n] =
                mix16(out_pcm[seek_pcm + n], pcm16->mono.pcm[n], pcm16->gain);
          seek_pcm += pcm16->mono.play_end;
        }
      } break;
      case 1: {
        uint32_t lseek_pcm = 0, rseek_pcm = 0;
        for (track_t *pcm16 = n->value.write.pcm; pcm16 != NULL;
             pcm16 = pcm16->next) {
          for (uint32_t n = 0; n < pcm16->stereo.lnb_samples; n++)
            out_pcm[lseek_pcm + n * 2] = mix16(
                out_pcm[lseek_pcm + n], pcm16->stereo.lpcm[n], pcm16->gain);
          for (uint32_t n = 0; n < pcm16->stereo.rnb_samples; n++)
            out_pcm[rseek_pcm + n * 2 + 1] = mix16(
                out_pcm[rseek_pcm + n], pcm16->stereo.rpcm[n], pcm16->gain);

          lseek_pcm += pcm16->stereo.lplay_end;
          rseek_pcm += pcm16->stereo.rplay_end;
        }
      }break;
      }
    fwrite(out_pcm, sizeof(int16_t), tnb_samples * wav.NumChannels, fp);
    free(out_pcm);
    fclose(fp);
    }
  }
  return 0;
}
