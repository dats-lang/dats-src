#ifndef DFILTER_H
#define DFILTER_H
#include "env.h"
#include <stdint.h>

/* increment MAJOR if changes were made to the DFILTER struct
 * and its members and set MINOR to 0 (zero). Like for example,
 * changes that would increase or decreases the size of DFILTER
 * struct.
 **/
#define DFILTER_API_VERSION_MAJOR 0

/* increment MINOR if changes were made to the API functions, but
 * not DFILTER struct nor its members itself.
**/
#define DFILTER_API_VERSION_MINOR 0

#define DFILTER_SET_API_VERSION                                                \
  .api_version.major = DFILTER_API_VERSION_MAJOR,                              \
  .api_version.minor = DFILTER_API_VERSION_MINOR

enum DFOptionType { DFOPTION_FLOAT, DFOPTION_STRING, DFOPTION_INT };
typedef enum DFOptionType DFOptionType;

typedef struct DFOption DFOption;
struct DFOption {
  DFOptionType type;
  const char *option_name;
  const char *description;
  union {
    char *strv;
    float floatv;
    int intv;
  } value;
};

typedef struct DFFilter DFFilter;
struct DFFilter {
  /* API version */
  struct {
    uint32_t major;
    uint32_t minor;
  } api_version;

  /* Filter name */
  const char *name;

  /* Short unique description of the filter */
  const char *description;

  /* Possible options to be set for your filter.
   * Set this to NULL if there are none */
  DFOption *const options;

  /* Prints documentation to stdout.
   * This must always point to something, not NULL! */
  void (*const print_documentation)(void);

  /* This is set to your implementation of your filter. */
  int (*filter)(track_t *pcm_dest, track_t *pcm_src);
};

#endif /* DFILTER_H */
