#include "wav.h"
#include <assert.h>
#include <env.h>
#include <stdint.h>
#include <stdlib.h>

extern int gen_track(dats_t *, track_t *);
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
    for (track_t *track = n->value.write.track; track != NULL;
         track = track->next)
      gen_track(dats, track);
    uint32_t tnb_samples = 0;
    for (track_t *track = n->value.write.track; track != NULL;
         track = track->next) {
      switch (track->track_type) {
      case 0:
          tnb_samples += track->mono.nb_samples; break;
      case 1:
        if (track->next != NULL) {
          tnb_samples += track->stereo.lplay_end;
        } else
          tnb_samples += track->stereo.lnb_samples;
#if 0
        if (track->next != NULL) {
          tnb_samples += track->stereo.rplay_end;
        } else
          tnb_samples += track->stereo.rnb_samples;
#endif
        break;
      }

      struct WAV_info wav = {
          .fp = fp,
          .Subchunk1Size = 16,
          .AudioFormat = 1,
          .NumChannels = (n->value.write.track->track_type) ? 2 : 1,
          .SampleRate = 44100,
          .NumSamples = /*(n->value.write.track->track_type ? 2 : 1)*/tnb_samples,
          .BitsPerSample = 16,
      };
      wav_write_header(&wav);
      int16_t *out_pcm = calloc(tnb_samples * wav.NumChannels, sizeof(int16_t));
      assert(out_pcm != NULL);

      switch (n->value.write.track->track_type) {
      case 0: {
        uint32_t seek_pcm = 0;
        for (track_t *track = n->value.write.track; track != NULL;
             track = track->next) {
          for (uint32_t n = 0; n < track->mono.nb_samples; n++)
            out_pcm[seek_pcm + n] =
                mix16(out_pcm[seek_pcm + n], track->mono.pcm[n], track->gain);
          seek_pcm += track->mono.play_end;
        }
      } break;
      case 1: {
        uint32_t lseek_pcm = 0, rseek_pcm = 0;
        for (track_t *track = n->value.write.track; track != NULL;
             track = track->next) {
          for (uint32_t n = 0; n < track->stereo.lnb_samples; n++){
            out_pcm[lseek_pcm + (n * 2)] = mix16(
                out_pcm[lseek_pcm + n*2], track->stereo.lpcm[n], track->gain);
            out_pcm[rseek_pcm + (n * 2) + 1] = mix16(
                out_pcm[rseek_pcm + n*2 + 1], track->stereo.rpcm[n], track->gain);
          }

          lseek_pcm += 2*track->stereo.lplay_end;
          rseek_pcm += (2*track->stereo.rplay_end) + 1;
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
