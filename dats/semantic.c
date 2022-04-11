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

void print_pcm16_t(pcm16_t *const pcm16);

static int duplicates_symrec_t(dats_t *d, symrec_t *sym) {
  for (symrec_t *sym1 = sym; sym1 != NULL; sym1 = sym1->next) {
    char *id1;
    switch (sym1->type) {
    case TOK_PCM16:
      id1 = sym1->value.pcm16.identifier;
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
      case TOK_PCM16:
        id2 = sym2->value.pcm16.identifier;
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

int semantic_pcm16_t(dats_t *d, symrec_t *sym, pcm16_t *pcm16_cur) {
  int err = 0;
  if (pcm16_cur == NULL)
    return 0;

  switch (pcm16_cur->type) {
  case ID: {
    symrec_t *pcm16 = getsym(d, pcm16_cur->ID.id);
    if (pcm16 == NULL) {
      REPORT("Undefined reference to '%s'\n", pcm16_cur->ID.id);
      print_scan_line(d->fp, pcm16_cur->ID.line, pcm16_cur->ID.column);
      goto exit;
    }
    if (pcm16->type != TOK_PCM16) {
      SEMANTIC(d, pcm16_cur->ID.line, pcm16_cur->ID.column,
               "Incompatible %s to pcm16\n", token_t_to_str(pcm16->type));
      REPORT("note: previously declared here:\n");
      print_scan_line(d->fp, pcm16->line, pcm16->column);
      goto exit;
    }
    if (pcm16 == sym) {
      SEMANTIC(d, pcm16_cur->ID.line, pcm16_cur->ID.column,
               "Infringing definition to itself\n");
      REPORT("with:\n");
      print_scan_line(d->fp, pcm16->line, pcm16->column);
    }
  }
    goto exit;
  case SYNTH: {
    const DSSynth *driver = get_dsynth_by_name(pcm16_cur->SYNTH.synth_name);
    if (driver == NULL) {
      SEMANTIC(d, pcm16_cur->SYNTH.synth_line, pcm16_cur->SYNTH.synth_column,
               "No synth named, '%s'\n", pcm16_cur->SYNTH.synth_name);
      err = 1;
    }
    symrec_t *pcm16 = getsym(d, pcm16_cur->SYNTH.staff_name);
    if (pcm16 == NULL) {
      SEMANTIC(d, pcm16_cur->SYNTH.staff_line, pcm16_cur->SYNTH.staff_column,
               "Undefined reference to '%s'\n", pcm16_cur->SYNTH.staff_name);
      err = 1;
    }
    if (err)
      goto exit;
    if (pcm16->type != TOK_STAFF) {
      SEMANTIC(d, pcm16_cur->SYNTH.staff_line, pcm16_cur->SYNTH.staff_column,
               "Incompatible %s to staff\n", token_t_to_str(pcm16->type));
      REPORT("note: previously declared here:\n");
      print_scan_line(d->fp, pcm16->line, pcm16->column);
    }
    for (size_t i = 0; i < pcm16_cur->SYNTH.nb_options; i++) {
      DSOption *options = NULL;
      for (options = driver->options; options->option_name != NULL; options++) {
        if (!strcmp(options->option_name,
                    pcm16_cur->SYNTH.options[i].option_name)) {
          goto found;
        }
      }
      SEMANTIC(d, pcm16_cur->SYNTH.options[i].line,
               pcm16_cur->SYNTH.options[i].column,
               "No synth options named, '%s'\n",
               pcm16_cur->SYNTH.options[i].option_name);
      continue;
    found : {}
      if (pcm16_cur->SYNTH.options[i].is_strv &&
          options->type == DSOPTION_FLOAT) {
        SEMANTIC(d, pcm16_cur->SYNTH.options[i].line,
                 pcm16_cur->SYNTH.options[i].column,
                 "Option, '%s', requires value\n",
                 pcm16_cur->SYNTH.options[i].option_name);
      } else if (!pcm16_cur->SYNTH.options[i].is_strv &&
                 options->type == DSOPTION_STRING) {
        SEMANTIC(d, pcm16_cur->SYNTH.options[i].line,
                 pcm16_cur->SYNTH.options[i].column,
                 "Option, '%s', requires string\n",
                 pcm16_cur->SYNTH.options[i].option_name);
      }
    }
    //    printf("Synth %s found\n", tok_identifier);
    //    free(tok_identifier);

    //    pcm16_t *(*const synth)(const symrec_t *const staff) =
    //    driver->synth;
  }
    goto exit;
  case FILTER: {
    const DFFilter *driver = get_dfilter_by_name(pcm16_cur->FILTER.filter_name);
    if (driver == NULL) {
      SEMANTIC(d, pcm16_cur->FILTER.filter_line,
               pcm16_cur->FILTER.filter_column, "No filter named, '%s'\n",
               pcm16_cur->FILTER.filter_name);
    }
    for (pcm16_t *pc = pcm16_cur->FILTER.pcm16_arg; pc != NULL; pc = pc->next)
      semantic_pcm16_t(d, sym, pc);
    // pcm16_cur = pcm16_cur->FILTER.pcm16_arg;
  }
    goto exit;
  case MIX:
    for (uint32_t nb_args = 0; nb_args < pcm16_cur->MIX.nb_pcm16; nb_args++)
      semantic_pcm16_t(d, sym, pcm16_cur->MIX.pcm16[nb_args]);
    goto exit;
  }
exit:
  return local_errors; // semantic_pcm16_t(d, n, c->next);
}

int semantic_cur_dats_t(dats_t *d) {
  int err = 0;
  local_errors = 0;
  duplicates_symrec_t(d, d->sym_table);
  for (symrec_t *n = d->sym_table; n != NULL; n = n->next) {
    switch (n->type) {
    case TOK_PCM16:
      // print_pcm16_t(n->value.pcm16.pcm);
      for (pcm16_t *pc = n->value.pcm16.pcm; pc != NULL; pc = pc->next)
        (void)semantic_pcm16_t(d, n, pc);
      break;
    case TOK_WRITE:
      (void)semantic_pcm16_t(d, n, n->value.write.pcm);
      break;
    }
  }

  if (local_errors)
    err = 1;
  global_errors += local_errors;
  local_errors = 0;

  return err;
}
