/* Dats interpreter
 *
 * Copyright (c) 2022 Al-buharie Amjari
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
 * Lesser General Public License for more details.v
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Dats; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <assert.h>
#include <string.h>

#include "env.h"
#include "scanner.h"

#include "libdfilter/allfilter.h"
#include "libdsynth/allsynth.h"

void print_track_t(track_t *const track);

static int duplicates_symrec_t(dats_t *d, symrec_t *sym) {
  for (symrec_t *sym1 = sym; sym1 != NULL; sym1 = sym1->next) {
    char *id1;
    switch (sym1->type) {
    case TOK_TRACK:
      id1 = sym1->value.track.identifier;
      break;
    case TOK_STAFF:
      id1 = sym1->value.staff.identifier;
      break;
    default:
      continue;
    }
    for (symrec_t *sym2 = sym1->next; sym2 != NULL; sym2 = sym2->next) {
      char *id2;
      switch (sym2->type) {
      case TOK_TRACK:
        id2 = sym2->value.track.identifier;
        break;
      case TOK_STAFF:
        id2 = sym2->value.staff.identifier;
        break;
      default:
        continue;
      }
      if (!strcmp(id1, id2)) {
        SEMANTIC(d, sym1->line, sym1->column, "Redefinition of '%s'\n", id1);
        REPORT("note: previously declared here:\n");
        print_scan_line(d->fp, sym2->line, sym2->column);
      }
    }
  }
  return 0;
}

int semantic_track_t(dats_t *d, symrec_t *sym, track_t *track_cur) {
  int err = 0;
  if (track_cur == NULL)
    return 0;

  switch (track_cur->type) {
  case ID: {
    symrec_t *track = getsym(d, track_cur->ID.id);
    if (track == NULL) {
      REPORT("Undefined reference to '%s'\n", track_cur->ID.id);
      print_scan_line(d->fp, track_cur->ID.line, track_cur->ID.column);
      goto exit;
    }
    if (track->type != TOK_TRACK) {
      SEMANTIC(d, track_cur->ID.line, track_cur->ID.column,
               "Incompatible %s to track\n", token_t_to_str(track->type));
      REPORT("note: previously declared here:\n");
      print_scan_line(d->fp, track->line, track->column);
      goto exit;
    }
    if (track == sym) {
      SEMANTIC(d, track_cur->ID.line, track_cur->ID.column,
               "Infringing definition to itself\n");
      REPORT("with:\n");
      print_scan_line(d->fp, track->line, track->column);
    }
  }
    goto exit;
  case SYNTH: {
    const DSSynth *driver = get_dsynth_by_name(track_cur->SYNTH.synth_name);
    if (driver == NULL) {
      SEMANTIC(d, track_cur->SYNTH.synth_line, track_cur->SYNTH.synth_column,
               "No synth named, '%s'\n", track_cur->SYNTH.synth_name);
      err = 1;
    }
    symrec_t *track = getsym(d, track_cur->SYNTH.staff_name);
    if (track == NULL) {
      SEMANTIC(d, track_cur->SYNTH.staff_line, track_cur->SYNTH.staff_column,
               "Undefined reference to '%s'\n", track_cur->SYNTH.staff_name);
      err = 1;
    }
    if (err)
      goto exit;
    if (track->type != TOK_STAFF) {
      SEMANTIC(d, track_cur->SYNTH.staff_line, track_cur->SYNTH.staff_column,
               "Incompatible %s to staff\n", token_t_to_str(track->type));
      REPORT("note: previously declared here:\n");
      print_scan_line(d->fp, track->line, track->column);
    }
    for (size_t i = 0; i < track_cur->SYNTH.nb_options; i++) {
      DSOption *options = NULL;
      for (options = driver->options; options->option_name != NULL; options++) {
        if (!strcmp(options->option_name,
                    track_cur->SYNTH.options[i].option_name)) {
          goto found;
        }
      }
      SEMANTIC(d, track_cur->SYNTH.options[i].line,
               track_cur->SYNTH.options[i].column,
               "No synth options named, '%s'\n",
               track_cur->SYNTH.options[i].option_name);
      continue;
    found : {}
      if (track_cur->SYNTH.options[i].is_strv &&
          options->type == DSOPTION_FLOAT) {
        SEMANTIC(d, track_cur->SYNTH.options[i].line,
                 track_cur->SYNTH.options[i].column,
                 "Option, '%s', requires value\n",
                 track_cur->SYNTH.options[i].option_name);
      } else if (!track_cur->SYNTH.options[i].is_strv &&
                 options->type == DSOPTION_STRING) {
        SEMANTIC(d, track_cur->SYNTH.options[i].line,
                 track_cur->SYNTH.options[i].column,
                 "Option, '%s', requires string\n",
                 track_cur->SYNTH.options[i].option_name);
      }
    }
    //    printf("Synth %s found\n", tok_identifier);
    //    free(tok_identifier);

    //    track_t *(*const synth)(const symrec_t *const staff) =
    //    driver->synth;
  }
    goto exit;
  case FILTER: {
    const DFFilter *driver = get_dfilter_by_name(track_cur->FILTER.filter_name);
    if (driver == NULL) {
      SEMANTIC(d, track_cur->FILTER.filter_line,
               track_cur->FILTER.filter_column, "No filter named, '%s'\n",
               track_cur->FILTER.filter_name);
    }
    for (track_t *pc = track_cur->FILTER.track_arg; pc != NULL; pc = pc->next)
      semantic_track_t(d, sym, pc);
    // track_cur = track_cur->FILTER.track_arg;
  }
    goto exit;
  case MIX:
    for (uint32_t nb_args = 0; nb_args < track_cur->MIX.nb_track; nb_args++)
      semantic_track_t(d, sym, track_cur->MIX.track[nb_args]);
    goto exit;
  }
exit:
  return local_errors; // semantic_track_t(d, n, c->next);
}

int semantic_cur_dats_t(dats_t *d) {
  int err = 0;
  local_errors = 0;
  duplicates_symrec_t(d, d->sym_table);
  for (symrec_t *n = d->sym_table; n != NULL; n = n->next) {
    switch (n->type) {
    case TOK_TRACK:
      // print_track_t(n->value.track.track);
      for (track_t *pc = n->value.track.track; pc != NULL; pc = pc->next)
        (void)semantic_track_t(d, n, pc);
      break;
    case TOK_WRITE:
      (void)semantic_track_t(d, n, n->value.write.track);
      break;
    }
  }

  if (local_errors)
    err = 1;
  global_errors += local_errors;
  local_errors = 0;

  return err;
}
