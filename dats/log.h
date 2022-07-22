#ifndef LOG_H
#define LOG_H

#include <stdio.h>

/* Assuming unix machines always supports ANSI color codes */
#ifdef __unix__
#define GREEN_ON "\x1b[1;32m"
#define RED_ON "\x1b[1;31m"
#define COLOR_OFF "\x1b[0m"
#else
#define GREEN_ON
#define RED_ON
#define COLOR_OFF
#endif

#define DATS_ERROR(...) fprintf(stderr, __VA_ARGS__)
#define DATS_VERROR(...)                                                       \
  {                                                                            \
    DATS_ERROR("[" GREEN_ON "%s:%d @ %s" COLOR_OFF "] ", __FILE__, __LINE__,   \
               __func__);                                                      \
    DATS_ERROR(__VA_ARGS__);                                                   \
  }

#endif /* LOG_H */
