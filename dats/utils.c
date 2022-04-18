#include "env.h"
#include <stdio.h>

void print_all_nr_t(nr_t *nr) {
  for (nr_t *p = nr; p != NULL; p = p->next) {
    switch (p->type) {
    case SYM_NOTE:
      printf("NOTE length: %u frequency: %f\n", p->length, p->note->frequency);
      break;
    case SYM_REST:
      printf("REST length: %u\n", p->length);
      break;
    }
  }
}

void print_track_t(track_t *const track) {
  if (track == NULL)
    return;
  puts("PCM16");
  int ctr = 0;
  for (track_t *p = track; p != NULL; p = p->next) {
    printf("%s: %d\n", __func__, ctr++);
    switch (p->type) {
    case ID:
      printf("ID %s\n", p->ID.id);
      break;
    case SYNTH:
      printf("SYNTH %s %s\n", p->SYNTH.synth_name, p->SYNTH.staff_name);
      for (size_t i = 0; i < p->SYNTH.nb_options; i++) {
        printf("option: %s\n", p->SYNTH.options[i].option_name);
        fflush(stdout);
      }
      break;
    case FILTER:
      printf("FILTER %s %p\n", p->FILTER.filter_name, p->FILTER.track_arg);
      print_track_t(p->FILTER.track_arg);
      for (size_t i = 0; i < p->FILTER.nb_options; i++) {
        printf("option: %s\n", p->FILTER.options[i].option_name);
        fflush(stdout);
      }
      break;
    }
  }
}
