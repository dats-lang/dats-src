/* Dats interpreter
 *
 * Copyright (c) 2021-2022 Al-buharie Amjari
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
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void print_quote(int, void *, void *);

#include "scanner.h"

extern void print_all_nr_t(nr_t *nr);

static token_t tok;
static symrec_t *staff;
static int rule_match = 0;
static dats_t *d;

/* Returns 0 if success. Non-zero if failed. */
static int parse_notes_rests() {
  if (tok == TOK_N) {
    nr_t *cnr = malloc(sizeof(nr_t));
    assert(cnr != NULL);
    cnr->type = SYM_NOTE;
    cnr->length = 0;
    cnr->next = NULL;
  add_lengthn:
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      free(cnr);
      return 1;
    }

    cnr->length += (uint32_t)(60.0 * 44100.0 * 4.0 / (tok_bpm * tok_num));
    uint32_t dotted_len =
        (uint32_t)(60.0 * 44100.0 * 4.0 / (tok_bpm * tok_num));
  checkn:
    tok = read_next_tok(d);
    switch (tok) {
    case TOK_DOT:
      dotted_len /= 2;
      cnr->length += dotted_len;
      goto checkn;
    case TOK_ADD:
      goto add_lengthn;
    case TOK_COMMA:
      break;
    default:
      free(cnr);
      UNEXPECTED(tok, d);
      return 1;
    }
    staff->value.staff.nb_samples += cnr->length;
    tok = read_next_tok(d);

    note_t *n = malloc(sizeof(note_t));
    note_t *f = n;
    assert(n != NULL);

    if (tok != TOK_NOTE) {
      free(cnr);
      free(n);
      UNEXPECTED(tok, d);
      return 1;
    }

  addn: /* add dyad */
    f->frequency = tok_num * pow(2.0, (double)tok_octave) *
                   pow(1.059463094, (double)tok_semitone);
    f->mnkey = tok_note + tok_semitone + tok_octave * 0x0c;

    f->attack = tok_attack;
    f->decay = tok_decay;
    f->sustain = tok_sustain;
    f->release = tok_release;
    f->volume = tok_volume;
    f->duration = cnr->length;
    tok = read_next_tok(d);

    switch (tok) {
    case TOK_DOT: {
      f->duration /= 2;
      tok = read_next_tok(d);
      break;
    }
    case TOK_UNDERSCORE: {
      f->duration /= 4;
      tok = read_next_tok(d);
      break;
    }
    case TOK_NOTE: {
      f->next = malloc(sizeof(note_t));
      assert(f != NULL);
      f = f->next;
      goto addn;
    }
    }

    f->next = NULL;
    cnr->note = n;

    if (staff->value.staff.nr != NULL) {
      for (nr_t *p = staff->value.staff.nr; 1; p = p->next) {
        if (p->next == NULL) {
          p->next = cnr;
          cnr->next = NULL;
          break;
        }
      }
    } else
      staff->value.staff.nr = cnr;
    rule_match = 1;
  } else if (tok == TOK_R) {
    nr_t *cnr = malloc(sizeof(nr_t));
    assert(cnr != NULL);
    cnr->type = SYM_REST;
    cnr->length = 0;
    cnr->next = NULL;
  add_lengthr:
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      UNEXPECTED(tok, d);
      free(cnr);
      return 1;
    }
    cnr->length += (uint32_t)(60.0 * 44100.0 * 4.0 / (tok_bpm * tok_num));
    uint32_t dotted_len =
        (uint32_t)(60.0 * 44100.0 * 4.0 / (tok_bpm * tok_num));
  checkr:
    tok = read_next_tok(d);
    switch (tok) {
    case TOK_DOT:
      dotted_len /= 2;
      cnr->length += dotted_len;
      goto checkr;
    case TOK_ADD:
      goto add_lengthr;
    default:;
    }
    staff->value.staff.nb_samples += cnr->length;

    if (staff->value.staff.nr != NULL)
      for (nr_t *p = staff->value.staff.nr; 1; p = p->next) {
        if (p->next == NULL) {
          p->next = cnr;
          break;
        }
      }
    else
      staff->value.staff.nr = cnr;
    rule_match = 1;
  } else if (tok == TOK_BPM) {
    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      UNEXPECTED(tok, d);
      return 1;
    }
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }
    if (tok_num == 0.0) {
      WARNING("0 bpm is illegal. Now setting to 120\n");
      tok_num = 120.0;
    }
    tok_bpm = tok_num;
    rule_match = 1;
    tok = read_next_tok(d);

  } else if (tok == TOK_VOLUME) {
    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      UNEXPECTED(tok, d);
      return 1;
    }
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }
    tok_volume = (int)tok_num;
    rule_match = 1;
    tok = read_next_tok(d);

  } else if (tok == TOK_ATTACK) {
    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      UNEXPECTED(tok, d);
      return 1;
    }
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }
    tok_attack = tok_num;
    rule_match = 1;
    tok = read_next_tok(d);

  } else if (tok == TOK_DECAY) {
    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      UNEXPECTED(tok, d);
      return 1;
    }
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }
    tok_decay = tok_num;
    rule_match = 1;
    tok = read_next_tok(d);

  } else if (tok == TOK_SUSTAIN) {
    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      UNEXPECTED(tok, d);
      return 1;
    }
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }
    tok_sustain = tok_num;
    rule_match = 1;
    tok = read_next_tok(d);

  } else if (tok == TOK_OCTAVE) {
    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      UNEXPECTED(tok, d);
      return 1;
    }
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }
    tok_octave = tok_num;
    if (tok_octave > 3 || tok_octave < -3) {
      C_ERROR(d, "Illegal range, (-3 to 3)");
      return 1;
    }
    rule_match = 1;
    tok = read_next_tok(d);

  } else if (tok == TOK_SEMITONE) {
    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      UNEXPECTED(tok, d);
      return 1;
    }
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }
    tok_semitone = tok_num;
    //    if (tok_semitone>1 || tok_semitone<-1){
    //      C_ERROR(d, "Illegal range, (-1 to 1)");
    //      return 1;
    //    }
    rule_match = 1;
    tok = read_next_tok(d);

  } else if (tok == TOK_RELEASE) {
    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      UNEXPECTED(tok, d);
      return 1;
    }
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }
    tok_release = tok_num;
    rule_match = 1;
    tok = read_next_tok(d);

  } else
    return 0;

  if (tok != TOK_SEMICOLON) {
    UNEXPECTED(tok, d);
    return 1;
  }
  tok = read_next_tok(d);
  return 0;
}

static int parse_staff() {
  tok_bpm = 120.0;
  tok_octave = 0;
  tok_semitone = 0;
  tok_volume = 10000;
  tok_attack = 300.0;
  tok_decay = 300.0;
  tok_sustain = 1.0;
  tok_release = 300.0;

  tok = read_next_tok(d);
  if (tok != TOK_IDENTIFIER) {
    EXPECTING(TOK_IDENTIFIER, d);
    return 1;
  }
  /* insert symrec_t */
  staff = malloc(sizeof(symrec_t));
  assert(staff != NULL);
  staff->type = TOK_STAFF;
  staff->line = line_token_found;
  staff->column = column_token_found;
  staff->value.staff.identifier = tok_identifier;
  tok_identifier = NULL;
  staff->value.staff.nb_samples = 0;
  staff->value.staff.nr = NULL;
  staff->next = d->sym_table;
  d->sym_table = staff;

  tok = read_next_tok(d);
  if (tok != TOK_LCURLY_BRACE) {
    UNEXPECTED(tok, d);
    return 1;
  }

  tok = read_next_tok(d);
  do {
    rule_match = 0;
    if (parse_notes_rests())
      return 1;
  } while (rule_match);

  if (tok != TOK_RCURLY_BRACE) {
    UNEXPECTED(tok, d);
    return 1;
  }
  return 0;
}

#define PARSE_PCM16_MAX_CALLS 64
static pcm16_t *parse_pcm16_tail(pcm16_t *pcm16_head, pcm16_t *pcm16_tail) {
  /* To prevent recursive calls which would eventually crash because of the
   * limitation of the stack, recursive calls are simulated by recording the
   * following variables:
   */
  int nb_calls = 0; /* The number of recursive calls */

  /* Its previous arguments */
  pcm16_t *prev_pcm16h[PARSE_PCM16_MAX_CALLS] = {NULL},
          *prev_pcm16t[PARSE_PCM16_MAX_CALLS] = {NULL};

  /* and where is its previous caller*/
  pcm16_type_t calls[PARSE_PCM16_MAX_CALLS] = {0};

append:
  if (nb_calls == PARSE_PCM16_MAX_CALLS) {
    C_ERROR(d, "%d maximum calls has reached. Killing self\n",
            PARSE_PCM16_MAX_CALLS);
    print_quote(1, NULL, NULL);
    pcm16_head = NULL;
    // longjmp(calls[nb_calls-1], 1);
  }

  tok = read_next_tok(d);
  switch (tok) {
  case TOK_SYNTH: {
    tok = read_next_tok(d);
    if (tok != TOK_DOT) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }
    tok = read_next_tok(d);
    if (tok != TOK_IDENTIFIER) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }
    pcm16_tail->type = SYNTH;
    pcm16_tail->pcm = NULL;
    pcm16_tail->nb_samples = 0;
    pcm16_tail->SYNTH.synth_line = line_token_found;
    pcm16_tail->SYNTH.synth_column = column_token_found;
    pcm16_tail->SYNTH.synth_name = tok_identifier;
    pcm16_tail->SYNTH.staff_name = NULL;
    pcm16_tail->SYNTH.options = NULL;
    pcm16_tail->SYNTH.nb_options = 0;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok != TOK_LPAREN) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }

    tok = read_next_tok(d);
    if (tok != TOK_IDENTIFIER) {
      EXPECTING(TOK_IDENTIFIER, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }
    pcm16_tail->SYNTH.staff_line = line_token_found;
    pcm16_tail->SYNTH.staff_column = column_token_found;
    pcm16_tail->SYNTH.staff_name = tok_identifier;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok != TOK_RPAREN) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }

    tok = read_next_tok(d);
    if (tok == TOK_LBRACKET) {
      size_t nb_options = 1;
      do {
        tok = read_next_tok(d);
        if (tok != TOK_IDENTIFIER) {
          C_ERROR(d, "Expecting options\n");
          destroy_pcm16_t(pcm16_head);
          return NULL;
        }
        const size_t option_size = sizeof(synth_option_t);
        pcm16_tail->SYNTH.options =
            realloc(pcm16_tail->SYNTH.options, option_size * nb_options);
        assert(pcm16_tail->SYNTH.options != NULL);
        pcm16_tail->SYNTH.options[nb_options - 1].option_name = tok_identifier;
        pcm16_tail->SYNTH.options[nb_options - 1].line = (int)line_token_found;
        pcm16_tail->SYNTH.options[nb_options - 1].column =
            (int)column_token_found;

        tok_identifier = NULL;
        tok = read_next_tok(d);
        if (tok != TOK_EQUAL) {
          EXPECTING(TOK_EQUAL, d);
          destroy_pcm16_t(pcm16_head);
          return NULL;
        }
        tok = read_next_tok(d);
        switch (tok) {
        case TOK_FLOAT:
          pcm16_tail->SYNTH.options[nb_options - 1].intv = (int)tok_num;
          pcm16_tail->SYNTH.options[nb_options - 1].floatv = tok_num;
          pcm16_tail->SYNTH.options[nb_options - 1].strv = NULL;
          printf("Driver num %f\n", tok_num);
          tok = read_next_tok(d);
          break;
        default:
          if (tok != TOK_DQUOTE) {
            C_ERROR(d, "Option expects a string, '\"'");
            destroy_pcm16_t(pcm16_head);
            return NULL;
          }
          expecting = TOK_STRING;
          tok = read_next_tok(d);
          expecting = TOK_NULL;
          pcm16_tail->SYNTH.options[nb_options - 1].strv = tok_identifier;
          printf("Driver num %s\n", tok_identifier);
          tok_identifier = NULL;
          tok = read_next_tok(d);
          if (tok != TOK_DQUOTE) {
            C_ERROR(d, "Strings must end with a double quote");
            destroy_pcm16_t(pcm16_head);
            return NULL;
          }
          tok = read_next_tok(d);
          break;
        }
        nb_options++;
      } while (tok == TOK_COMMA);
      pcm16_tail->SYNTH.nb_options = nb_options - 1;
      if (tok != TOK_RBRACKET) {
        EXPECTING(TOK_RBRACKET, d);
        destroy_pcm16_t(pcm16_head);
        return NULL;
      }
      tok = read_next_tok(d);
    }
    if (tok == TOK_COMMA) {
      pcm16_tail->next = malloc(sizeof(pcm16_t));
      assert(pcm16_tail != NULL);
      pcm16_tail = pcm16_tail->next;
      pcm16_tail->next = NULL;
      goto append;
    }
  } break;
  case TOK_IDENTIFIER: {
    pcm16_tail->type = ID;
    pcm16_tail->ID.line = line_token_found;
    pcm16_tail->ID.column = column_token_found;
    pcm16_tail->pcm = NULL;
    pcm16_tail->ID.id = tok_identifier;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok == TOK_COMMA) {
      pcm16_tail->next = malloc(sizeof(pcm16_t));
      assert(pcm16_tail != NULL);
      pcm16_tail = pcm16_tail->next;
      pcm16_tail->next = NULL;
      goto append;
    }
  } break;
  case TOK_MIX: {
    tok = read_next_tok(d);
    if (tok != TOK_LPAREN) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }
    pcm16_tail->type = MIX;
    pcm16_tail->pcm = NULL;
    pcm16_tail->MIX.line = line_token_found;
    pcm16_tail->MIX.column = column_token_found;
    pcm16_tail->MIX.nb_pcm16 = 0;
    pcm16_tail->MIX.pcm16 = NULL;
    int nb_pcm16s = 0;
    pcm16_t **pcm16s = NULL;

    do {
      tok = read_next_tok(d);
      if (tok != TOK_LPAREN) {
        UNEXPECTED(tok, d);
        destroy_pcm16_t(pcm16_head);
        return NULL;
      }
      nb_pcm16s++;
      pcm16s = realloc(pcm16s, sizeof(pcm16_t *) * nb_pcm16s);
      assert(pcm16s != NULL);
      pcm16s[nb_pcm16s - 1] = malloc(sizeof(pcm16_t));
      assert(pcm16s[nb_pcm16s - 1] != NULL);
      pcm16s[nb_pcm16s - 1]->next = NULL;

      prev_pcm16h[nb_calls] = pcm16_head;
      prev_pcm16t[nb_calls] = pcm16_tail;
      pcm16_head = pcm16s[nb_pcm16s - 1];
      pcm16_tail = pcm16s[nb_pcm16s - 1];
      calls[nb_calls] = MIX;
      nb_calls++;
      tok_identifier = NULL;
      /* call the function */
      goto append;
    MIX:
      nb_calls--;
      /* Restore this function arguments */
      pcm16_tail = prev_pcm16t[nb_calls];
      pcm16s[nb_pcm16s - 1] = pcm16_head;
      pcm16_head = prev_pcm16h[nb_calls];

      if (tok != TOK_RPAREN) {
        UNEXPECTED(tok, d);
        destroy_pcm16_t(pcm16_head);
        return NULL;
      }
      tok = read_next_tok(d);
    } while (tok == TOK_COMMA);
    if (tok != TOK_RPAREN) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }
    pcm16_tail->MIX.pcm16 = pcm16s;
    pcm16_tail->MIX.nb_pcm16 = nb_pcm16s;
    tok = read_next_tok(d);

    // printf("read %s\n", token_t_to_str(tok));
    if (tok == TOK_COMMA) {
      pcm16_tail->next = malloc(sizeof(pcm16_t));
      assert(pcm16_tail != NULL);
      pcm16_tail = pcm16_tail->next;
      pcm16_tail->next = NULL;
      goto append;
    }
  } break;
  case TOK_FILTER: {
    tok = read_next_tok(d);
    if (tok != TOK_DOT) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }
    tok = read_next_tok(d);
    if (tok != TOK_IDENTIFIER) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }
    pcm16_tail->type = FILTER;
    pcm16_tail->pcm = NULL;
    pcm16_tail->FILTER.filter_line = line_token_found;
    pcm16_tail->FILTER.filter_column = column_token_found;
    pcm16_tail->FILTER.filter_name = tok_identifier;
    pcm16_tail->FILTER.pcm16_arg = NULL;
    pcm16_tail->FILTER.options = NULL;
    pcm16_tail->FILTER.nb_options = 0;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok != TOK_LPAREN) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }

    pcm16_tail->FILTER.pcm16_line = line_token_found;
    pcm16_tail->FILTER.pcm16_column = column_token_found;
    pcm16_tail->FILTER.pcm16_arg = malloc(sizeof(pcm16_t));
    assert(pcm16_tail->FILTER.pcm16_arg != NULL);
    pcm16_tail->FILTER.pcm16_arg->next = NULL;

    /* Push current state to stack */
    prev_pcm16h[nb_calls] = pcm16_head;
    prev_pcm16t[nb_calls] = pcm16_tail;

    /* arguments */
    pcm16_head = pcm16_tail->FILTER.pcm16_arg;
    pcm16_tail = pcm16_tail->FILTER.pcm16_arg;
    //    if (!setjmp(calls[nb_calls])) {
    //      nb_calls++;
    //      tok_identifier = NULL;
    //      goto append;
    //    }
    calls[nb_calls] = FILTER;
    nb_calls++;
    tok_identifier = NULL;
    /* call the function */
    goto append;
  FILTER:
    /* Pop the stack and restore previous state */
    nb_calls--;
    pcm16_tail = prev_pcm16t[nb_calls];
    pcm16_tail->FILTER.pcm16_arg = pcm16_head;
    assert(pcm16_head != NULL);
    pcm16_head = prev_pcm16h[nb_calls];
    // parse_pcm16_tail(
    // pcm16_tail->FILTER.pcm16_arg, pcm16_tail->FILTER.pcm16_arg);

    if (pcm16_tail->FILTER.pcm16_arg == NULL) {
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }

    if (tok != TOK_RPAREN) {
      UNEXPECTED(tok, d);
      destroy_pcm16_t(pcm16_head);
      return NULL;
    }

    tok = read_next_tok(d);
    if (tok == TOK_LBRACKET) {
      size_t nb_options = 1;
      do {
        tok = read_next_tok(d);
        if (tok != TOK_IDENTIFIER) {
          C_ERROR(d, "Expecting options\n");
          destroy_pcm16_t(pcm16_head);
          return NULL;
        }
        const size_t option_size = sizeof(filter_option_t);
        pcm16_tail->FILTER.options =
            realloc(pcm16_tail->FILTER.options, option_size * nb_options);
        assert(pcm16_tail->FILTER.options != NULL);
        pcm16_tail->FILTER.options[nb_options - 1].option_name = tok_identifier;
        tok_identifier = NULL;
        tok = read_next_tok(d);
        if (tok != TOK_EQUAL) {
          EXPECTING(TOK_EQUAL, d);
          destroy_pcm16_t(pcm16_head);
          return NULL;
        }
        tok = read_next_tok(d);
        switch (tok) {
        case TOK_FLOAT:
          pcm16_tail->FILTER.options[nb_options - 1].intv = (int)tok_num;
          pcm16_tail->FILTER.options[nb_options - 1].floatv = tok_num;
          pcm16_tail->FILTER.options[nb_options - 1].strv = NULL;
          printf("Driver num %f\n", tok_num);
          tok = read_next_tok(d);
          break;
        default:
          if (tok != TOK_DQUOTE) {
            C_ERROR(d, "Option expects a string, '\"'");
            destroy_pcm16_t(pcm16_head);
            return NULL;
          }
          expecting = TOK_STRING;
          tok = read_next_tok(d);
          expecting = TOK_NULL;
          pcm16_tail->FILTER.options[nb_options - 1].strv = tok_identifier;
          printf("Driver num %s\n", tok_identifier);
          tok_identifier = NULL;
          tok = read_next_tok(d);
          if (tok != TOK_DQUOTE) {
            C_ERROR(d, "Strings must end with a double quote");
            destroy_pcm16_t(pcm16_head);
            return NULL;
          }
          tok = read_next_tok(d);
          break;
        }
        nb_options++;
      } while (tok == TOK_COMMA);
      pcm16_tail->FILTER.nb_options = nb_options - 1;
      if (tok != TOK_RBRACKET) {
        EXPECTING(TOK_RBRACKET, d);
        destroy_pcm16_t(pcm16_head);
        return NULL;
      }
      tok = read_next_tok(d);
    }
    if (tok == TOK_COMMA) {
      pcm16_tail->next = malloc(sizeof(pcm16_t));
      assert(pcm16_tail != NULL);
      pcm16_tail = pcm16_tail->next;
      pcm16_tail->next = NULL;
      goto append;
    }
  } break;
  default:
    UNEXPECTED(tok, d);
    destroy_pcm16_t(pcm16_head);
    return NULL;
  }
  if (nb_calls != 0) {
    switch (calls[nb_calls - 1]) {
    case FILTER:
      goto FILTER;
    case MIX:
      goto MIX;
    }
    // longjmp(calls[nb_calls - 1], 1);
  }
  return pcm16_head;
}

static symrec_t *parse_pcm16(char *id) {
  symrec_t *pcm16 = malloc(sizeof(symrec_t));
  assert(pcm16 != NULL);
  pcm16->type = TOK_PCM16;
  pcm16->line = line_token_found;
  pcm16->column = column_token_found;
  pcm16->value.pcm16.nb_samples = 0;
  pcm16->value.pcm16.identifier = id;
  pcm16->value.pcm16.pcm = NULL;
  pcm16->next = d->sym_table;
  d->sym_table = pcm16;

  pcm16_t *pcm16_head = malloc(sizeof(pcm16_t));
  assert(pcm16_head != NULL);
  pcm16_head->next = NULL;
  pcm16_head->pcm = NULL;

  tok_identifier = NULL;
  if (parse_pcm16_tail(pcm16_head, pcm16_head) == NULL)
    return NULL;
  pcm16->value.pcm16.pcm = pcm16_head;
  return pcm16;
}

static int parse_stmt() {
  /* statements */
  if (tok == TOK_PCM16) {
    tok = read_next_tok(d);
    if (tok != TOK_IDENTIFIER) {
      UNEXPECTED(tok, d);
      return 1;
    }
    size_t line = line_token_found, column = column_token_found;
    char *id = tok_identifier;
    symrec_t *pcm16 = NULL;

    tok = read_next_tok(d);
    if (tok != TOK_EQUAL) {
      free(id);
      if (tok == TOK_IDENTIFIER)
        free(tok_identifier);
      tok_identifier = NULL;
      UNEXPECTED(tok, d);
      return 1;
    }

    line_token_found = line;
    column_token_found = column;
    pcm16 = parse_pcm16(id);
    if (pcm16 == NULL)
      return 1;
    rule_match = 1;
  } else if (tok == TOK_WRITE) {
    tok = read_next_tok(d);
    if (tok != TOK_LPAREN) {
      EXPECTING(TOK_LPAREN, d);
      return 1;
    }

    tok = read_next_tok(d);
    if (tok != TOK_DQUOTE) {
      C_ERROR(d, "`write`, expects an identifier between double quote\n");
      return 1;
    }
    expecting = TOK_STRING;
    tok = read_next_tok(d);
    if (tok != TOK_STRING) {
      EXPECTING(TOK_STRING, d);
      return 1;
    }
    expecting = TOK_NULL;
    /*
         FILE *fp = fopen(tok_identifier, "wb");
         if (fp == NULL) {
           perror(tok_identifier);
           C_ERROR(d, "ERROR!\n");
           return 1;
         }
    */
    symrec_t *write = malloc(sizeof(symrec_t));
    assert(write != NULL);
    write->type = TOK_WRITE;
    write->line = line_token_found;
    write->column = column_token_found;
    //write->value.write.nb_samples = 0;
    write->value.write.out_file = tok_identifier;
    tok_identifier = NULL;
    write->value.write.pcm = NULL;
    write->next = d->sym_table;
    d->sym_table = write;

    pcm16_t *pcm16_head = malloc(sizeof(pcm16_t));
    assert(pcm16_head != NULL);
    pcm16_head->next = NULL;
    pcm16_head->pcm = NULL;

    write->value.write.pcm = pcm16_head;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok != TOK_DQUOTE) {
      C_ERROR(d, "Identifier must end with a double quote\n");
      return 1;
    }

    tok = read_next_tok(d);
    if (tok != TOK_COMMA) {
      EXPECTING(TOK_COMMA, d);
      return 1;
    }
    if (parse_pcm16_tail(pcm16_head, pcm16_head) == NULL)
      return 1;

    /*
         symrec_t *pcm16 = parse_pcm16(NULL);
         if (pcm16 == NULL)
           return 1;
         if (pcm16->type != TOK_PCM16) {
           C_ERROR(d, "Macro, `write`, needs pcm16 variable type\n");
           return 1;
         }
         struct WAV_info wav = {
             .fp = fp,
             .Subchunk1Size = 16,
             .AudioFormat = 1,
             .NumChannels = 1,
             .SampleRate = 44100,
             .NumSamples = pcm16->value.pcm16.total_numsamples,
             .BitsPerSample = 16,
         };
         wav_write_header(&wav);
         for (pcm16_t *ptmp = pcm16->value.pcm16.pcm; ptmp != NULL;
              ptmp = ptmp->next)
           fwrite(ptmp->pcm, sizeof(int16_t), ptmp->numsamples, fp);
         fclose(fp);
    */

    if (tok != TOK_RPAREN) {
      EXPECTING(TOK_RPAREN, d);
      return 1;
    }

    tok = read_next_tok(d);
    rule_match = 1;
  } else
    return 0;

  if (tok != TOK_SEMICOLON) {
    UNEXPECTED(tok, d);
    return 1;
  }

  tok = read_next_tok(d);

  return 0;
}

static int parse_master() {

  tok = read_next_tok(d);
  if (tok != TOK_LCURLY_BRACE) {
    UNEXPECTED(tok, d);
    return 1;
  }

  tok = read_next_tok(d);
  do {
    rule_match = 0;
    if (parse_stmt())
      return 1;
  } while (rule_match);

  if (tok != TOK_RCURLY_BRACE) {
    UNEXPECTED(tok, d);
    return 1;
  }

  return 0;
}

static int start() {

  switch (tok) {
  case TOK_STAFF:
    do {
      if (parse_staff())
        return 1;
    } while (tok == TOK_STAFF);
    return local_errors;
  case TOK_MAIN:
    do {
      if (parse_master())
        return 1;
    } while (tok == TOK_MAIN);
    return local_errors;
  case TOK_EOF:
    return local_errors;
  default:
    UNEXPECTED(tok, d);
    return 1;
  }

  return local_errors;
}

/* Returns 0 if success. Non-zero if failed. */
int parse_cur_dats_t(dats_t *const t) {
  local_errors = 0;
  d = t;

  while (1) {
    tok = read_next_tok(d);
    if (start()) {
      ERROR("%d local errors generated\n", local_errors);
      global_errors += local_errors;
      local_errors = 0;
      free(tok_identifier);
      tok_identifier = NULL;
      return 1;
    }
    if (tok == TOK_EOF)
      break;
  }
  /*printf("[" GREEN_ON "%s:%d @ %s" COLOR_OFF "] %s: parsing successful\n",
         __FILE__, __LINE__, __func__, d->fname);+*/

  return local_errors;
}
