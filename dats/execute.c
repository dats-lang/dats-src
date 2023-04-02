#include "log.h"
#include "wav.h"
#include <assert.h>
#include <env.h>
#include <stdint.h>
#include <stdlib.h>

extern int gen_track(dats_t *, track_t *);
extern int16_t mixs16(int16_t, int16_t, float);

int execute_dats_t(dats_t *dats) {
  symrec_t *main = NULL;
  for (main = dats->sym_table; main != NULL; main = main->next)
    if (main->type == TOK_MAIN)
      break;

  if (main == NULL) {
    DATS_VERROR("No main\n");
    return 1;
  }

  for (symrec_t *n = main->value.main.stmt; n != NULL; n = n->next) {
    switch (n->type) {
    case TOK_TRACK:
      gen_track(dats, n->value.track.track);
      continue;
    case TOK_WRITE: {
      FILE *fp = fopen(n->value.write.out_file, "wb");
      if (fp == NULL) {
        perror(n->value.write.out_file);
        return 1;
      }

      /* makes sure tracks are initialized before writing */
      for (track_t *track = n->value.write.track; track != NULL;
           track = track->next)
        gen_track(dats, track);

      uint32_t tnb_samples = 0;
      for (track_t *track = n->value.write.track; track != NULL;
           track = track->next) {
        switch (track->track_type) {
        case 0:
          if (track->next != NULL) {
            tnb_samples += track->mono.play_end;
          } else
            tnb_samples += track->mono.nb_samples;
          continue;
        case 1:
          if (track->next != NULL) {
            tnb_samples += (track->stereo.lplay_end > track->stereo.rplay_end)
                               ? track->stereo.lplay_end
                               : track->stereo.lplay_end;
          } else
            tnb_samples += track->stereo.lnb_samples;
          continue;
        }
      }

      struct WAV_info wav = {
          .fp = fp,
          .Subchunk1Size = 16,
          .AudioFormat = 1,
          .NumChannels = (n->value.write.track->track_type) ? 2 : 1,
          .SampleRate = 44100,
          .NumSamples = tnb_samples,
          .BitsPerSample = 16,
      };
      wav_write_header(&wav);

      int16_t *out_pcm = calloc(tnb_samples * wav.NumChannels, sizeof(int16_t));
      assert(out_pcm != NULL);

      switch (n->value.write.track->track_type) {
      case 0: {
        /* FIXME handle when apended tracks is mono or stereo */
        uint32_t seek_pcm = 0;
        for (track_t *track = n->value.write.track; track != NULL;
             track = track->next) {
          for (uint32_t n = 0; n < track->mono.nb_samples; n++)
            out_pcm[seek_pcm + n] =
                mixs16(out_pcm[seek_pcm + n], track->mono.pcm[n], track->gain);
          seek_pcm += track->mono.play_end;
        }
      } break;
      case 1: {
        /* FIXME handle when apended tracks is mono or stereo */
        uint32_t lseek_pcm = 0, rseek_pcm = 0;
        for (track_t *track = n->value.write.track; track != NULL;
             track = track->next) {
          for (uint32_t n = 0; n < track->stereo.lnb_samples; n++) {
            out_pcm[lseek_pcm + (n * 2)] = mixs16(
                out_pcm[lseek_pcm + n * 2], track->stereo.lpcm[n], track->gain);
            out_pcm[rseek_pcm + (n * 2) + 1] =
                mixs16(out_pcm[rseek_pcm + n * 2 + 1], track->stereo.rpcm[n],
                      track->gain);
          }

          lseek_pcm += 2 * track->stereo.lplay_end;
          rseek_pcm += 2 * track->stereo.rplay_end;
        }
      } break;
      } /* switch (n->value.write.track->track_type) */

      fwrite(out_pcm, sizeof(int16_t), tnb_samples * wav.NumChannels, fp);
      free(out_pcm);
      fclose(fp);

    } break; /* TOK_WRITE */
    }
  }

  return 0;
}
