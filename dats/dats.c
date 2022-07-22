/*  Dats interpreter
 *
 * Copyright (c) 2021 Al-buharie Amjari
 *
 * This file is part of Dats.
 *
 * Dats is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Dats is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Dats; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "libdfilter/allfilter.h"
#include "libdsynth/allsynth.h"
#include "scanner.h"
#include "version.h"

/* Parses dats_t */
extern int parse_cur_dats_t(dats_t *const t);
extern int semantic_cur_dats_t(dats_t const *const);
extern int exec_write(dats_t *);

char **synth_paths = NULL;
int synth_paths_nb = 0;

char **filter_paths = NULL;
int filter_paths_nb = 0;

int enable_debug = 0;
/* process_args returns the value 0 if sucesss and nonzero if
 * failed.
 */

static void init_paths(void) {
  /* init synth paths */
  synth_paths = malloc(sizeof(char *));
  assert(synth_paths != NULL);

  synth_paths[0] = calloc(1, 2);
  assert(synth_paths[0] != NULL);
  strcat(synth_paths[0], ".");

  synth_paths_nb = 1;

  /* init filter paths */
  filter_paths = malloc(sizeof(char *));
  assert(filter_paths != NULL);

  filter_paths[0] = calloc(1, 2);
  assert(filter_paths[0] != NULL);
  strcat(filter_paths[0], ".");

  filter_paths_nb = 1;
}

static int process_args(const int argc, char *const *argv) {
  if (argc == 1) {
    DATS_ERROR("No argument supplied!\nUse --help to print help\n");
    return 1;
  }
  FILE *fp;
  for (int i = 1; i < argc; i++) {
    switch (argv[i][0]) {
    case '-':
      if (argv[i][1] == '-') {
        if (!strcmp(&argv[i][2], "help")) {
          /* clang-format off */
          printf("Dats interpreter Draft-2.0.0 ");
          putchar('(');
      {   int i = 0;
          while (i!=____git_refs_heads_master_len-1)
          putchar(____git_refs_heads_master[i++]);
#ifdef DUMMY_COMMIT
          printf("-dummy");
#endif
      }   putchar(':');
      {   int i = 0;
          while (i!=____git_HEAD_len-1)
          putchar((____git_HEAD+5)[i++]);
      }
          puts(")");
          printf("%s [dats file]\noptions:\n\n", argv[0]);
          printf("--list-synths                        prints all available synths\n"
                 "--list-filters                       prints all available filters\n");
          /* clang-format on */
          return 0;
        } else if (!strcmp(&argv[i][2], "list-synths")) {
          print_synths();
          return 0;
        } else if (!strcmp(&argv[i][2], "list-filters")) {
          print_filters();
          return 0;
        }
      }
      if (argv[i][1] == 'd' && !argv[i][2]) {
        enable_debug = 1;
        continue;
      }
      if (argv[i][1] == 'S') {
        synth_paths = realloc(synth_paths, sizeof(char *)*(synth_paths_nb + 1));
        assert(synth_paths != NULL);
        size_t len = strlen(&argv[i][2]);
        synth_paths[synth_paths_nb] = malloc(len + 1);
        strcpy(synth_paths[synth_paths_nb], &argv[i][2]);
        synth_paths_nb++;
        continue;
      }
      if (argv[i][1] == 'F') {
        filter_paths = realloc(filter_paths, sizeof(char *)*(filter_paths_nb + 1));
        assert(filter_paths != NULL);
        size_t len = strlen(&argv[i][2]);
        filter_paths[filter_paths_nb] = malloc(len + 1);
        strcpy(filter_paths[filter_paths_nb], &argv[i][2]);
        filter_paths_nb++;
        continue;
      }

      DATS_ERROR(RED_ON "error" COLOR_OFF ": unknown option '%s'\n", argv[i]);
      global_errors++;
      break;
    default:
      fp = fopen(argv[i], "rb");
      if (fp == NULL) {
        perror(argv[i]);
        return 1;
      }
      dats_t *p = malloc(sizeof(dats_t));
      assert(p != NULL);
      p->fp = fp;
      p->fname = strdup(argv[i]);
      assert(p->fname != NULL);
      p->line = 1;
      p->column = 0;
      p->sym_table = NULL;
      p->next = dats_files;

      if (fgets(p->scan_line, 500, p->fp) != NULL)
        fseek(p->fp, -(long)(strlen(p->scan_line)), SEEK_CUR);

      dats_files = p;
      break;
    }
  }

  return global_errors;
}

int main(int argc, char **argv) {
#ifdef _WIN32
  {
    HINSTANCE dll_module;
    if ((dll_module = LoadLibrary("exchndl.dll")) != NULL) {
      /* This is a security hole as anyone can create dll and put
       * whatever they want to ExcHndlInit. Verify that the dll is genuine
       * and came from legitimate sites or github account ye user..
       */
      void (*exchndl_init)(void) = GetProcAddress(dll_module, "ExcHndlInit");
      exchndl_init();
    }
  }
#endif

  init_paths();
  int ret;
  ret = process_args(argc, argv);
  if (ret)
    goto err;

  /* Individually parse each dats_t* */
  for (dats_t *p = dats_files; p != NULL; p = p->next) {
    /* if parse current dats_t returns non zero, then it
     * must be skipped
     */
    if (parse_cur_dats_t(p))
      continue;
    if (semantic_cur_dats_t(p))
      continue;
    exec_write(p);
    print_all_symrec_t_cur_dats_t(p);
  }
  if (global_errors)
    goto err;
err:
  if (global_errors)
    DATS_ERROR("\n%d global errors generated\n", global_errors);

  clean_all_symrec_t_all_dats_t();
  clean_all_dats_t();

  for (int i = 0; i < synth_paths_nb; i++)
    free(synth_paths[i]);
  free(synth_paths);

  return global_errors ? 1 : 0;
}
