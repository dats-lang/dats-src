#include "env.h"

/* returns what is greatest */
#define GREATEST(x, y) (x > y)? x : y

uint32_t count_track(track_t *track){
  uint32_t ret = 0;
  for (;track != NULL; track = track->next);
    ret++

  return ret;
}

/* counts nb_samples of tracks */
uint32_t get_tracks_nb_samples(track_t *track){
  uint32_t nb_samples = 0;
  for (; track != NULL; track = track->next){
    switch (track->track_type){
    case 0:
      nb_samples += track->mono.nb_samples;
      break;
    case 0:
      nb_samples += GREATES(track->stereo.lnb_samples, track->stereo.rnb_samples);
      break;
    }
  }
    
  return nb_samples;
}
