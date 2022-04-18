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

#define MAX_BLOCK                                                              \
  42 /* The answer to life, universe and                                       \
      * everything.                                                            \
      */
/* Returns 0 if success. Non-zero if failed. */
static int parse_notes_rests(bnr_t *blk) {
  static int nb_blk = 0;
  static bnr_t *prev_blk[MAX_BLOCK] = {0};

add_block:
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
    blk->nb_samples += cnr->length;
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

    if (blk->nr != NULL) {
      for (nr_t *p = blk->nr; 1; p = p->next) {
        if (p->next == NULL) {
          p->next = cnr;
          cnr->next = NULL;
          break;
        }
      }
    } else
      blk->nr = cnr;
    rule_match = 1;

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }
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
    // staff->value.staff.nb_samples += cnr->length;
    blk->nb_samples += cnr->length;

    if (blk->nr != NULL)
      for (nr_t *p = blk->nr; 1; p = p->next) {
        if (p->next == NULL) {
          p->next = cnr;
          break;
        }
      }
    else
      blk->nr = cnr;
    rule_match = 1;

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }
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

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }

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

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }

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

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }

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

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }

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

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }

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

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }

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

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }

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

    if (tok != TOK_SEMICOLON) {
      UNEXPECTED(tok, d);
      return 1;
    }

  } else if (tok == TOK_REPEAT) {
    tok = read_next_tok(d);
    if (tok != TOK_FLOAT) {
      EXPECTING(TOK_FLOAT, d);
      return 1;
    }

    tok = read_next_tok(d);
    if (tok != TOK_LCURLY_BRACE) {
      EXPECTING(TOK_LCURLY_BRACE, d);
      return 1;
    }

    bnr_t *block = malloc(sizeof(bnr_t));
    assert(block != NULL);
    block->block_repeat = (uint8_t)tok_num;
    block->nb_samples = 0;
    block->nr = NULL;
    prev_blk[nb_blk] = blk;
    nb_blk++;

    nr_t *cnr = malloc(sizeof(nr_t));
    assert(cnr != NULL);
    cnr->type = SYM_BLOCK;
    cnr->block = block;
    cnr->next = NULL;

    if (blk->nr != NULL)
      for (nr_t *p = blk->nr; 1; p = p->next) {
        if (p->next == NULL) {
          p->next = cnr;
          break;
        }
      }
    else
      blk->nr = cnr;

    blk = block;
    tok = read_next_tok(d);
    goto add_block;

  end_block:
    nb_blk--;
    prev_blk[nb_blk]->nb_samples +=
        (uint32_t)(blk->block_repeat + 1) * blk->nb_samples;
    blk = prev_blk[nb_blk];

  } else
    return 0;

  tok = read_next_tok(d);
  if (nb_blk != 0 && tok == TOK_RCURLY_BRACE)
    goto end_block;
  if (nb_blk != 0)
    goto add_block;
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
  staff->value.staff.bnr = NULL;
  staff->next = d->sym_table;
  d->sym_table = staff;

  bnr_t *block = malloc(sizeof(bnr_t));
  assert(block != NULL);
  block->block_repeat = 0;
  block->nb_samples = 0;
  block->nr = NULL;

  staff->value.staff.bnr = block;

  tok = read_next_tok(d);
  if (tok != TOK_LCURLY_BRACE) {
    UNEXPECTED(tok, d);
    return 1;
  }

  tok = read_next_tok(d);
  do {
    rule_match = 0;
    if (parse_notes_rests(block))
      return 1;
  } while (rule_match);

  staff->value.staff.nb_samples =
      (uint32_t)(block->block_repeat + 1) * block->nb_samples;

  if (tok != TOK_RCURLY_BRACE) {
    UNEXPECTED(tok, d);
    return 1;
  }
  return 0;
}

#define PARSE_PCM16_MAX_CALLS 64
static track_t *parse_track_tail(track_t *track_head, track_t *track_tail) {
  /* To prevent recursive calls which would eventually crash because of the
   * limitation of the stack, recursive calls are simulated by recording the
   * following variables:
   */
  int nb_calls = 0; /* The number of recursive calls */

  /* Its previous arguments */
  track_t *prev_trackh[PARSE_PCM16_MAX_CALLS] = {NULL},
          *prev_trackt[PARSE_PCM16_MAX_CALLS] = {NULL};

  /* and where is its previous caller*/
  track_type_t calls[PARSE_PCM16_MAX_CALLS] = {0};

  char track_type = track_head->track_type;
append:
  if (nb_calls == PARSE_PCM16_MAX_CALLS) {
    C_ERROR(d, "%d maximum calls has reached. Killing self\n",
            PARSE_PCM16_MAX_CALLS);
    print_quote(1, NULL, NULL); /* self killed */
  }

  track_tail->track_type = track_type;
  switch (track_tail->track_type) {
  case 0:
    track_tail->mono.pcm = NULL;
    track_tail->mono.nb_samples = 0;
    break;
  case 1:
    track_tail->stereo.lpcm = NULL;
    track_tail->stereo.rpcm = NULL;
    track_tail->stereo.rnb_samples = 0;
    track_tail->stereo.lnb_samples = 0;
    break;
  }
  track_tail->next = NULL;

  tok = read_next_tok(d);
  if (tok == TOK_FLOAT) {
    track_tail->gain = tok_num;
    tok = read_next_tok(d);
  } else
    track_tail->gain = 1.0;

  switch (tok) {
  case TOK_SYNTH: {
    tok = read_next_tok(d);
    if (tok != TOK_DOT) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }
    tok = read_next_tok(d);
    if (tok != TOK_IDENTIFIER) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }
    track_tail->type = SYNTH;
    track_tail->SYNTH.synth_line = line_token_found;
    track_tail->SYNTH.synth_column = column_token_found;
    track_tail->SYNTH.synth_name = tok_identifier;
    track_tail->SYNTH.staff_name = NULL;
    track_tail->SYNTH.options = NULL;
    track_tail->SYNTH.nb_options = 0;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok != TOK_LPAREN) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }

    tok = read_next_tok(d);
    if (tok != TOK_IDENTIFIER) {
      EXPECTING(TOK_IDENTIFIER, d);
      destroy_track_t(track_head);
      return NULL;
    }
    track_tail->SYNTH.staff_line = line_token_found;
    track_tail->SYNTH.staff_column = column_token_found;
    track_tail->SYNTH.staff_name = tok_identifier;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok != TOK_RPAREN) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }

    tok = read_next_tok(d);
    if (tok == TOK_LBRACKET) {
      size_t nb_options = 1;
      do {
        tok = read_next_tok(d);
        if (tok != TOK_IDENTIFIER) {
          C_ERROR(d, "Expecting options\n");
          destroy_track_t(track_head);
          return NULL;
        }
        const size_t option_size = sizeof(synth_option_t);
        track_tail->SYNTH.options =
            realloc(track_tail->SYNTH.options, option_size * nb_options);
        assert(track_tail->SYNTH.options != NULL);
        track_tail->SYNTH.options[nb_options - 1].option_name = tok_identifier;
        track_tail->SYNTH.options[nb_options - 1].line = (int)line_token_found;
        track_tail->SYNTH.options[nb_options - 1].column =
            (int)column_token_found;

        tok_identifier = NULL;
        tok = read_next_tok(d);
        if (tok != TOK_EQUAL) {
          EXPECTING(TOK_EQUAL, d);
          destroy_track_t(track_head);
          return NULL;
        }
        tok = read_next_tok(d);
        switch (tok) {
        case TOK_FLOAT:
          track_tail->SYNTH.options[nb_options - 1].floatv = tok_num;
          track_tail->SYNTH.options[nb_options - 1].is_strv = 0;
          printf("Driver num %f\n", tok_num);
          tok = read_next_tok(d);
          break;
        default:
          if (tok != TOK_DQUOTE) {
            C_ERROR(d, "Option expects a string, '\"'");
            destroy_track_t(track_head);
            return NULL;
          }
          expecting = TOK_STRING;
          tok = read_next_tok(d);
          expecting = TOK_NULL;
          track_tail->SYNTH.options[nb_options - 1].strv = tok_identifier;
          track_tail->SYNTH.options[nb_options - 1].is_strv = 1;
          printf("Driver num %s\n", tok_identifier);
          tok_identifier = NULL;
          tok = read_next_tok(d);
          if (tok != TOK_DQUOTE) {
            C_ERROR(d, "Strings must end with a double quote");
            destroy_track_t(track_head);
            return NULL;
          }
          tok = read_next_tok(d);
          break;
        }
        nb_options++;
      } while (tok == TOK_COMMA);
      track_tail->SYNTH.nb_options = nb_options - 1;
      if (tok != TOK_RBRACKET) {
        EXPECTING(TOK_RBRACKET, d);
        destroy_track_t(track_head);
        return NULL;
      }
      tok = read_next_tok(d);
    }
    if (tok == TOK_COMMA) {
      track_tail->next = malloc(sizeof(track_t));
      assert(track_tail != NULL);
      track_tail = track_tail->next;
      track_tail->next = NULL;
      goto append;
    }
  } break;
  case TOK_IDENTIFIER: {
    track_tail->type = ID;
    track_tail->ID.line = line_token_found;
    track_tail->ID.column = column_token_found;
    track_tail->ID.id = tok_identifier;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok == TOK_COMMA) {
      track_tail->next = malloc(sizeof(track_t));
      assert(track_tail != NULL);
      track_tail = track_tail->next;
      track_tail->next = NULL;
      goto append;
    }
  } break;
  case TOK_MIX: {
    tok = read_next_tok(d);
    if (tok != TOK_LPAREN) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }
    track_tail->type = MIX;
    track_tail->MIX.line = line_token_found;
    track_tail->MIX.column = column_token_found;
    track_tail->MIX.nb_track = 0;
    track_tail->MIX.track = NULL;
    // track_t **tracks = NULL;

    do {
      tok = read_next_tok(d);
      if (tok != TOK_LPAREN) {
        UNEXPECTED(tok, d);
        destroy_track_t(track_head);
        return NULL;
      }
      track_tail->MIX.nb_track++;
      track_tail->MIX.track = realloc(
          track_tail->MIX.track, sizeof(track_t *) * track_tail->MIX.nb_track);
      assert(track_tail->MIX.track != NULL);
      track_tail->MIX.track[track_tail->MIX.nb_track - 1] =
          malloc(sizeof(track_t));
      assert(track_tail->MIX.track[track_tail->MIX.nb_track - 1] != NULL);
      track_tail->MIX.track[track_tail->MIX.nb_track - 1]->track_type =
          track_type;
      switch (track_tail->MIX.track[track_tail->MIX.nb_track - 1]->track_type) {
      case 0:
        track_tail->MIX.track[track_tail->MIX.nb_track - 1]->mono.pcm = NULL;
        track_tail->MIX.track[track_tail->MIX.nb_track - 1]->mono.nb_samples =
            0;
        break;
      case 1:
        track_tail->MIX.track[track_tail->MIX.nb_track - 1]->stereo.lpcm = NULL;
        track_tail->MIX.track[track_tail->MIX.nb_track - 1]->stereo.rpcm = NULL;
        track_tail->MIX.track[track_tail->MIX.nb_track - 1]
            ->stereo.rnb_samples = 0;
        track_tail->MIX.track[track_tail->MIX.nb_track - 1]
            ->stereo.lnb_samples = 0;
        break;
      }

      track_tail->MIX.track[track_tail->MIX.nb_track - 1]->next = NULL;

      prev_trackh[nb_calls] = track_head;
      prev_trackt[nb_calls] = track_tail;
      track_head = track_tail->MIX.track[track_tail->MIX.nb_track - 1];
      track_tail = track_tail->MIX.track[track_tail->MIX.nb_track - 1];
      calls[nb_calls] = MIX;
      nb_calls++;
      tok_identifier = NULL;
      /* call the function */
      goto append;
    MIX:
      nb_calls--;
      /* Restore this function arguments */
      track_tail = prev_trackt[nb_calls];
      track_tail->MIX.track[track_tail->MIX.nb_track - 1] = track_head;
      track_head = prev_trackh[nb_calls];

      if (tok != TOK_RPAREN) {
        UNEXPECTED(tok, d);
        destroy_track_t(track_head);
        return NULL;
      }
      tok = read_next_tok(d);
    } while (tok == TOK_COMMA);
    if (tok != TOK_RPAREN) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }
    track_tail->MIX.track = track_tail->MIX.track;
    tok = read_next_tok(d);

    if (tok == TOK_COMMA) {
      track_tail->next = malloc(sizeof(track_t));
      assert(track_tail != NULL);
      track_tail = track_tail->next;
      track_tail->next = NULL;
      goto append;
    }
  } break;
  case TOK_FILTER: {
    tok = read_next_tok(d);
    if (tok != TOK_DOT) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }
    tok = read_next_tok(d);
    if (tok != TOK_IDENTIFIER) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }
    track_tail->type = FILTER;
    track_tail->track_type = track_type;
    track_tail->FILTER.filter_line = line_token_found;
    track_tail->FILTER.filter_column = column_token_found;
    track_tail->FILTER.filter_name = tok_identifier;
    track_tail->FILTER.track_arg = NULL;
    track_tail->FILTER.options = NULL;
    track_tail->FILTER.nb_options = 0;
    tok_identifier = NULL;

    tok = read_next_tok(d);
    if (tok != TOK_LPAREN) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }

    track_tail->FILTER.track_line = line_token_found;
    track_tail->FILTER.track_column = column_token_found;
    track_tail->FILTER.track_arg = malloc(sizeof(track_t));
    assert(track_tail->FILTER.track_arg != NULL);
    track_tail->FILTER.track_arg->track_type = track_type;
    switch (track_tail->FILTER.track_arg->track_type) {
    case 0:
      track_tail->FILTER.track_arg->mono.pcm = NULL;
      track_tail->FILTER.track_arg->mono.nb_samples = 0;
      break;
    case 1:
      track_tail->FILTER.track_arg->stereo.lpcm = NULL;
      track_tail->FILTER.track_arg->stereo.rpcm = NULL;
      track_tail->FILTER.track_arg->stereo.lnb_samples = 0;
      track_tail->FILTER.track_arg->stereo.rnb_samples = 0;
      break;
    }
    track_tail->FILTER.track_arg->next = NULL;

    /* Push current state to stack */
    prev_trackh[nb_calls] = track_head;
    prev_trackt[nb_calls] = track_tail;

    /* arguments */
    track_head = track_tail->FILTER.track_arg;
    track_tail = track_tail->FILTER.track_arg;
    calls[nb_calls] = FILTER;
    nb_calls++;

    tok_identifier = NULL;
    /* call the function */
    goto append;
  FILTER:
    /* Pop the stack and restore previous state */
    nb_calls--;
    track_tail = prev_trackt[nb_calls];
    track_tail->FILTER.track_arg = track_head;
    assert(track_head != NULL);
    track_head = prev_trackh[nb_calls];

    if (track_tail->FILTER.track_arg == NULL) {
      destroy_track_t(track_head);
      return NULL;
    }

    if (tok != TOK_RPAREN) {
      UNEXPECTED(tok, d);
      destroy_track_t(track_head);
      return NULL;
    }

    tok = read_next_tok(d);
    if (tok == TOK_LBRACKET) {
      size_t nb_options = 1;
      do {
        tok = read_next_tok(d);
        if (tok != TOK_IDENTIFIER) {
          C_ERROR(d, "Expecting options\n");
          destroy_track_t(track_head);
          return NULL;
        }
        const size_t option_size = sizeof(filter_option_t);
        track_tail->FILTER.options =
            realloc(track_tail->FILTER.options, option_size * nb_options);
        assert(track_tail->FILTER.options != NULL);
        track_tail->FILTER.options[nb_options - 1].option_name = tok_identifier;
        tok_identifier = NULL;
        tok = read_next_tok(d);
        if (tok != TOK_EQUAL) {
          EXPECTING(TOK_EQUAL, d);
          destroy_track_t(track_head);
          return NULL;
        }
        tok = read_next_tok(d);
        switch (tok) {
        case TOK_FLOAT:
          track_tail->FILTER.options[nb_options - 1].floatv = tok_num;
          track_tail->FILTER.options[nb_options - 1].is_strv = 0;
          printf("Driver num %f\n", tok_num);
          tok = read_next_tok(d);
          break;
        default:
          if (tok != TOK_DQUOTE) {
            C_ERROR(d, "Option expects a string, '\"'");
            destroy_track_t(track_head);
            return NULL;
          }
          expecting = TOK_STRING;
          tok = read_next_tok(d);
          expecting = TOK_NULL;
          track_tail->FILTER.options[nb_options - 1].strv = tok_identifier;
          track_tail->FILTER.options[nb_options - 1].is_strv = 1;
          printf("Driver num %s\n", tok_identifier);
          tok_identifier = NULL;
          tok = read_next_tok(d);
          if (tok != TOK_DQUOTE) {
            C_ERROR(d, "Strings must end with a double quote");
            destroy_track_t(track_head);
            return NULL;
          }
          tok = read_next_tok(d);
          break;
        }
        nb_options++;
      } while (tok == TOK_COMMA);
      track_tail->FILTER.nb_options = nb_options - 1;
      if (tok != TOK_RBRACKET) {
        EXPECTING(TOK_RBRACKET, d);
        destroy_track_t(track_head);
        return NULL;
      }
      tok = read_next_tok(d);
    }
    if (tok == TOK_COMMA) {
      track_tail->next = malloc(sizeof(track_t));
      assert(track_tail != NULL);
      track_tail = track_tail->next;
      track_tail->next = NULL;
      goto append;
    }
  } break;
  default:
    UNEXPECTED(tok, d);
    /* print stack */
    for (int i = 0; i < nb_calls; i++) {
      switch (calls[i]) {
      case FILTER:
        printf("[debug] stack #%d filter\n", i);
        break;
      case MIX:
        printf("[debug] stack #%d mix\n", i);
        break;
      }
    }
    destroy_track_t(track_head);
    return NULL;
  }
  if (nb_calls != 0) {
    switch (calls[nb_calls - 1]) {
    case FILTER:
      goto FILTER;
    case MIX:
      goto MIX;
    }
  }
  return track_head;
}

static symrec_t *parse_track(char *id, token_t track_type) {
  symrec_t *track = malloc(sizeof(symrec_t));
  assert(track != NULL);
  track->type = TOK_TRACK;
  track->line = line_token_found;
  track->column = column_token_found;
  track->value.track.nb_samples = 0;
  track->value.track.identifier = id;
  track->value.track.track = NULL;
  track->next = d->sym_table;
  d->sym_table = track;

  track_t *track_head = malloc(sizeof(track_t));
  assert(track_head != NULL);
  track_head->track_type = (track_type == TOK_MONO) ? 0 : 1;
  switch (track_head->track_type) {
  case 0:
    track_head->mono.pcm = NULL;
    track_head->mono.nb_samples = 0;
    break;
  case 1:
    track_head->stereo.lpcm = NULL;
    track_head->stereo.rpcm = NULL;
    track_head->stereo.lnb_samples = 0;
    track_head->stereo.rnb_samples = 0;
    break;
  }
  track_head->next = NULL;

  tok_identifier = NULL;
  if (parse_track_tail(track_head, track_head) == NULL)
    return NULL;
  track->value.track.track = track_head;
  return track;
}

static int parse_stmt() {
  token_t track_type = TOK_STEREO;
  if (tok == TOK_MONO || tok == TOK_STEREO) {
    track_type = tok;
    tok = read_next_tok(d);
  }

  /* statements */
  if (tok == TOK_TRACK) {
    tok = read_next_tok(d);
    if (tok != TOK_IDENTIFIER) {
      UNEXPECTED(tok, d);
      return 1;
    }
    size_t line = line_token_found, column = column_token_found;
    char *id = tok_identifier;
    symrec_t *track = NULL;

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
    track = parse_track(id, track_type);
    if (track == NULL)
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

    symrec_t *write = malloc(sizeof(symrec_t));
    assert(write != NULL);
    write->type = TOK_WRITE;
    write->line = line_token_found;
    write->column = column_token_found;
    // write->value.write.nb_samples = 0;
    write->value.write.out_file = tok_identifier;
    tok_identifier = NULL;
    write->value.write.track = NULL;
    write->next = d->sym_table;
    d->sym_table = write;

    track_t *track_head = malloc(sizeof(track_t));
    assert(track_head != NULL);
    track_head->next = NULL;
    track_head->track_type = 1;
    track_head->stereo.lpcm = NULL;
    track_head->stereo.rpcm = NULL;
    track_head->stereo.lnb_samples = 0;
    track_head->stereo.rnb_samples = 0;

    write->value.write.track = track_head;
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
    if (parse_track_tail(track_head, track_head) == NULL)
      return 1;

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
