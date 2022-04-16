#ifndef DSYNTH_H
#define DSYNTH_H
#include "env.h"
#include <stdint.h>

enum DSOptionType { DSOPTION_FLOAT, DSOPTION_STRING };
typedef enum DSOptionType DSOptionType;

typedef struct DSOption DSOption;
struct DSOption {
  DSOptionType type;
  const char *option_name;
  const char *description;
  union {
    char *strv;
    float floatv;
  } value;
};

typedef struct DSSynth DSSynth;
struct DSSynth {
  /* Synth name */
  const char *name;

  /* Short unique description of the synth */
  const char *description;

  /* Possible options to be set for your synth.
   * Set this to NULL if there are none */
  DSOption *const options;

  /* Prints documentation on stdout.
   * This must always point to something, not NULL! */
  void (*const print_documentation)(void);

  /* This is set to your implementation of synth. */
  int (*const synth)(const symrec_t *const staff, track_t *const ctx);
};

#endif /* DSYNTH_H */
