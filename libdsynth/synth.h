#ifndef DSYNTH_H
#define DSYNTH_H
#include "env.h"
#include <stdint.h>

/* increment MAJOR if changes were made to the DSYNTH struct
 * and its members and set MINOR to 0 (zero). Like for example,
 * changes that would increase or decreases the size of DSYNTH
 * struct.
 **/
#define DSYNTH_API_VERSION_MAJOR 0

/* increment MINOR if changes were made to the API functions, but
 * not DSYNTH struct nor its members itself.
**/
#define DSYNTH_API_VERSION_MINOR 0

#define DSYNTH_SET_API_VERSION                                                \
  .api_version.major = DSYNTH_API_VERSION_MAJOR,                              \
  .api_version.minor = DSYNTH_API_VERSION_MINOR

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
  /* API version */
  struct {
    uint32_t major;
    uint32_t minor;
  } api_version;

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
