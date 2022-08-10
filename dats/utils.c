#include "env.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

extern const char *token_t_to_str(const token_t t);

extern char **synth_paths;
extern int synth_paths_nb;

extern char **filter_paths;
extern int filter_paths_nb;

void print_all_nr_t(nr_t *nr) {
  for (nr_t *p = nr; p != NULL; p = p->next) {
    switch (p->type) {
    case SYM_NOTE:
      printf("NOTE length: %u frequency: %f\n", p->length, p->note->frequency);
      break;
    case SYM_REST:
      printf("REST length: %u\n", p->length);
      break;
    case SYM_BLOCK:
      printf("BLOCK\n");
      break;
    }
  }
}

char *basename(const char *name) {
#ifdef _WIN32
  return strrchr(name, '\\');
#else
  return strrchr(name, '/');
#endif
}

void locate_synth(char *dest, const char *name, size_t n) {
  for (int i = 0; i < synth_paths_nb; i++) {
    char path[256] = {0};
    strncat(path, synth_paths[i], 255);
    strncat(path, "/", 255);
    strncat(path, name, 255);
    FILE *fp = fopen(path, "rb");
    if (fp != NULL) {
      strncpy(dest, path, n);
      return;
    }
  }

  memset(dest, 0, n);
}

void locate_filter(char *dest, const char *name, size_t n) {
  for (int i = 0; i < filter_paths_nb; i++) {
    char path[256] = {0};
    strncat(path, filter_paths[i], 255);
    strncat(path, "/", 255);
    strncat(path, name, 255);
    FILE *fp = fopen(path, "rb");
    if (fp != NULL) {
      strncpy(dest, path, n);
      return;
    }
  }

  memset(dest, 0, n);
}

void print_track_t(track_t *const track) {
  if (track == NULL)
    return;
  puts("TRACK");
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
    case MIX:
      printf("MIX %d\n", p->MIX.nb_track);
      for (uint32_t i = 0; i < p->MIX.nb_track; i++) {
        printf("arg %d: %p\n", i, p->MIX.track[i]);
        fflush(stdout);
        break;
      }
    }
  }
}

static void _print_symrec_t(symrec_t *const t) {
  for (symrec_t *p = t; p != NULL; p = p->next) {
    switch (p->type) {
    case TOK_STAFF:
      printf("  %-20s    %-20s\n", p->value.staff.identifier,
             token_t_to_str(TOK_STAFF));
      break;
    case TOK_TRACK:
      printf("  %-20s    %-20s %s\n",
             p->value.track.identifier == NULL ? "(null)"
                                               : p->value.track.identifier,
             token_t_to_str(TOK_TRACK),
             (!p->value.track.track->track_type) ? "mono" : "stereo");
      break;
    case TOK_WRITE:
      printf("  [write]\n");
      break;
    case TOK_MAIN:
      _print_symrec_t(p->value.main.stmt);
      break;
    default:
      DATS_VERROR("Unknown token\n");
    }
  }

}

/* prints the symbol table of the current dats_t* t*/
void print_dats_t(const dats_t *const t) {
  printf("Symbol table of %s\n%-20s    %-20s\n\n", t->fname, "  IDENTIFIER",
         "  TYPE");
  _print_symrec_t(t->sym_table);
}
