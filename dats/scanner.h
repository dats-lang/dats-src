#ifndef SCANNER_H
#define SCANNER_H
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#ifdef DEFINE_SCANNER_VARIABLES
#define EXTERN
#else
#define EXTERN extern
#endif

#include "env.h"

EXTERN void clean_all_dats_t(void);
/*
EXTERN void clean_all_symrec_t_cur_dats_t (const dats_t * const t);*/

EXTERN void clean_all_symrec_t_all_dats_t(void);

EXTERN int count_dats_t(void);
EXTERN const char *token_t_to_str(const token_t t);
EXTERN symrec_t *getsym(const dats_t *const t, char const *const id);
EXTERN token_t read_next_tok(dats_t *const t);
//EXTERN token_t peek_next_tok(dats_t *const t, int);
EXTERN void print_all_symrec_t_cur_dats_t(const dats_t *const t);
EXTERN void print_debugging_info(const token_t tok, dats_t *d);
EXTERN void print_scan_line(FILE *, const uint32_t, const uint32_t);
EXTERN void destroy_track_t(track_t *);

EXTERN uint32_t line_token_found;
EXTERN uint32_t column_token_found;
EXTERN int local_errors;
EXTERN int global_errors;
EXTERN int local_warnings;
EXTERN int global_warnings;

EXTERN int seek;
EXTERN float tok_num;
EXTERN float tok_bpm;
EXTERN float tok_attack;
EXTERN float tok_decay;
EXTERN float tok_sustain;
EXTERN float tok_release;
EXTERN int tok_note;
EXTERN int tok_volume;
EXTERN int tok_octave;
EXTERN int tok_semitone;
EXTERN char *tok_identifier;
EXTERN token_t expecting;

EXTERN dats_t *dats_files;

/* since Windows terminal doesn't support ANSI color codes */
#ifdef __unix__
#define GREEN_ON "\x1b[1;32m"
#define RED_ON "\x1b[1;31m"
#define COLOR_OFF "\x1b[0m"
#else
#define GREEN_ON
#define RED_ON
#define COLOR_OFF
#endif

#define ERROR(...) fprintf(stderr, __VA_ARGS__)

#define UNEXPECTED(x, d)                                                       \
  {                                                                            \
    local_errors++;                                                            \
    if (x != TOK_ERR) {                                                        \
      ERROR("[" GREEN_ON "%s:%d @ %s" COLOR_OFF "] %s:%d:%d " RED_ON           \
            "error" COLOR_OFF ": unexpected %s",                               \
            __FILE__, __LINE__, __func__, d->fname, line_token_found,          \
            column_token_found, token_t_to_str(x));                            \
      print_debugging_info(x, d);                                              \
    }                                                                          \
  }

#define WARNING(str)                                                           \
  ERROR("[" GREEN_ON "%s:%d @ %s" COLOR_OFF "] %s:%d:%d " RED_ON               \
        "warning" COLOR_OFF ": %s",                                            \
        __FILE__, __LINE__, __func__, d->fname, line_token_found,              \
        column_token_found + 1, str)

#define SEMANTIC(d, line, column, ...)                                         \
  {                                                                            \
    local_errors++;                                                            \
    ERROR("[" GREEN_ON "%s:%d @ %s" COLOR_OFF "] %s:%d:%d " RED_ON             \
          "error" COLOR_OFF ": ",                                              \
          __FILE__, __LINE__, __func__, d->fname, line, column);               \
    ERROR(__VA_ARGS__);                                                        \
    print_scan_line(d->fp, line, column);                                      \
  }

#define C_ERROR(d, ...)                                                        \
  {                                                                            \
    local_errors++;                                                            \
    ERROR("[" GREEN_ON "%s:%d@ %s" COLOR_OFF "] %s:%" PRIu32                   \
          ":%" PRIu32 RED_ON "error" COLOR_OFF ": ",                           \
          __FILE__, __LINE__, __func__, d->fname, line_token_found,            \
          column_token_found);                                                 \
    ERROR(__VA_ARGS__);                                                        \
    print_debugging_info(TOK_NULL, d);                                         \
  }

#define EXPECTING(x, d)                                                        \
  {                                                                            \
    local_errors++;                                                            \
    if (x != TOK_ERR)                                                          \
      ERROR("[" GREEN_ON "%s:%d @ %s" COLOR_OFF "] %s:%" PRIu32                \
            ":%" PRIu32 RED_ON "error" COLOR_OFF ": expecting %s",             \
            __FILE__, __LINE__, __func__, d->fname, line_token_found,          \
            column_token_found, token_t_to_str(x));                            \
    print_debugging_info(TOK_NULL, d);                                         \
  }

#define REPORT(...)                                                            \
  {                                                                            \
    ERROR("[" GREEN_ON "%s:%d @ %s" COLOR_OFF "] ", __FILE__, __LINE__,        \
          __func__);                                                           \
    ERROR(__VA_ARGS__);                                                        \
  }

#endif /* SCANNER_H */
