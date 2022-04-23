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

#ifndef _WIN32
extern void register_deadquote(void);
#endif

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

/* process_args returns the value 0 if sucesss and nonzero if
 * failed.
 */
int process_args(const int argc, char *const *argv) {
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
#if 0 /*__SANITIZE_ADDRESS__ != 1*/
  register_deadquote();
#endif
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
  return global_errors ? 1 : 0;
}
