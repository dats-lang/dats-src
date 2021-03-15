/* Dats compiler
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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "scanner.h"

#include "libsynth/allsynth.h"

extern void print_all_nr_t (nr_t * nr);

static token_t tok;
static symrec_t *staff;
static int rule_match = 0;
dats_t *d;

/* Returns 0 if success. Non-zero if failed. */
static int
parse_notes_rests ()
{
  if (tok == TOK_BPM)
    {
      tok = read_next_tok_cur_dats_t (d);
      if (tok != TOK_EQUAL)
        UNEXPECTED (tok, d);
      tok = read_next_tok_cur_dats_t (d);
      if (tok != TOK_FLOAT)
        EXPECTING (TOK_FLOAT, d);
      if (tok_num == 0.0)
        {
          WARNING ("0 bpm is illegal. Now setting to 120\n");
          tok_num = 120.0;
        }
      tok_bpm = tok_num;
      rule_match = 1;
    }
  else if (tok == TOK_N)
    {
      nr_t *cnr = malloc (sizeof (nr_t));
      assert (cnr != NULL);
      cnr->type = SYM_NOTE;
      cnr->next = NULL;

      tok = read_next_tok_cur_dats_t (d);
      if (tok != TOK_FLOAT)
        {
          EXPECTING (TOK_FLOAT, d);
        }

      cnr->length = (uint32_t) (60.0 * 44100.0 * 4.0 / (tok_bpm * tok_num));
      staff->value.staff.numsamples += cnr->length;
      tok = read_next_tok_cur_dats_t (d);

      if (tok != TOK_NOTE && tok != TOK_FLOAT)
        {
          UNEXPECTED (tok, d);
        }

      note_t *n = malloc (sizeof (note_t));
      assert (n != NULL);
      n->frequency = tok_num;
      cnr->note = n;

      if (staff->value.staff.nr != NULL)
        {
          for (nr_t * p = staff->value.staff.nr; 1; p = p->next)
            {
              if (p->next == NULL)
                {
                  p->next = cnr;
                  printf ("[debug] add note %p\n", cnr);
                  break;
                }
            }
        }
      else
        staff->value.staff.nr = cnr;
      rule_match = 1;
    }
  else if (tok == TOK_R)
    {
      nr_t *cnr = malloc (sizeof (nr_t));
      assert (cnr != NULL);
      cnr->type = SYM_REST;
      cnr->next = NULL;
      tok = read_next_tok_cur_dats_t (d);
      if (tok != TOK_FLOAT)
        {
          UNEXPECTED (tok, d);
        }
      cnr->length = (uint32_t) (60.0 * 44100.0 * 4.0 / (tok_bpm * tok_num));
      staff->value.staff.numsamples += cnr->length;
      if (staff->value.staff.nr != NULL)
        {
          for (nr_t * p = staff->value.staff.nr; 1; p = p->next)
            {
              if (p->next == NULL)
                {
                  p->next = cnr;
                  printf ("[debug] add rest %p\n", cnr);
                  break;
                }
            }
        }
      else
        staff->value.staff.nr = cnr;
      rule_match = 1;
    }
  else
    return 0;


  tok = read_next_tok_cur_dats_t (d);
  if (tok != TOK_SEMICOLON)
    {
      EXPECTING (TOK_SEMICOLON, d);
    }
  tok = read_next_tok_cur_dats_t (d);
  return 0;
}

static int
parse_staff ()
{
  tok_bpm = 120.0;              //default
  tok = read_next_tok_cur_dats_t (d);
  if (tok != TOK_IDENTIFIER)
    {
      EXPECTING (TOK_IDENTIFIER, d);
    }
  printf ("=== BEGIN STAFF === %s\n", tok_identifier);
  /* insert symrec_t */
  staff = malloc (sizeof (symrec_t));
  assert (staff != NULL);
  staff->type = TOK_STAFF;
  staff->line = line_token_found;
  staff->column = column_token_found;
  staff->value.staff.identifier = strdup (tok_identifier);
  free (tok_identifier);
  tok_identifier = NULL;
  staff->value.staff.numsamples = 0;
  staff->value.staff.nr = NULL;
  staff->next = d->sym_table;
  d->sym_table = staff;

  tok = read_next_tok_cur_dats_t (d);
  if (tok != TOK_LCURLY_BRACE)
    {
      UNEXPECTED (tok, d);
    }

  tok = read_next_tok_cur_dats_t (d);
  do
    {
      rule_match = 0;
      if (parse_notes_rests ())
        return 1;
    }
  while (rule_match);

  if (tok != TOK_RCURLY_BRACE)
    {
      UNEXPECTED (tok, d);
    }
  puts ("=== END STAFF ===");
/*
  ERROR ("num parser %d\n", staff->value.staff.numsamples);
  print_all_nr_t (staff->value.staff.nr);*/
  rule_match = 1;
  return 0;
}

static int
parse_track ()
{
  if (tok == TOK_PCM16)
    {
      tok = read_next_tok_cur_dats_t (d);
      if (tok != TOK_IDENTIFIER)
        UNEXPECTED (tok, d);
      symrec_t *pcm16 = malloc (sizeof (symrec_t));
      pcm16->type = TOK_PCM16;
      pcm16->value.pcm16.identifier = tok_identifier;
      pcm16->value.pcm16.pcm = NULL;
      tok_identifier = NULL;
      pcm16->next = d->sym_table;
      d->sym_table = pcm16;

      tok = read_next_tok_cur_dats_t (d);
      if (tok != TOK_EQUAL)
        UNEXPECTED (tok, d);

      tok = read_next_tok_cur_dats_t (d);
      switch (tok)
        {
        case TOK_SYNTH:
          tok = read_next_tok_cur_dats_t (d);
          if (tok != TOK_DOT)
            UNEXPECTED (tok, d);
          tok = read_next_tok_cur_dats_t (d);
          if (tok != TOK_IDENTIFIER)
            UNEXPECTED (tok, d);

          const DSynth *synth = get_dsynth_by_name (tok_identifier);
          if (synth == NULL)
            C_ERROR ("No synth %s\n", tok_identifier);
          printf ("Synth %s found\n", tok_identifier);
          free (tok_identifier);
          tok_identifier = NULL;
          break;

        default:
          C_ERROR ("Undefined reference to \"%s\"\n", tok_identifier);
        }
      tok = read_next_tok_cur_dats_t (d);
    }                           /* 
                                   else if (tok == TOK_TRACK)
                                   {
                                   symrec_t *a = getsym (d, "master");
                                   {
                                   master_t *m = malloc (sizeof (master_t));
                                   m->next = a->value.master;
                                   m->track = NULL;
                                   a->value.master = m;
                                   }
                                   do
                                   {
                                   tok = read_next_tok_cur_dats_t (d);
                                   if (tok != TOK_IDENTIFIER)
                                   break;

                                   symrec_t *s = getsym (d, tok_identifier);
                                   if (s == NULL)
                                   {
                                   local_errors++;
                                   C_ERROR ("undefined reference to \"%s\"\n", tok_identifier);
                                   }
                                   free (tok_identifier);
                                   tok_identifier = NULL;
                                   if (s->type != TOK_SAMPLE)
                                   {
                                   local_errors++;
                                   C_ERROR ("Dats forbids the use of non staff in track\n");
                                   }

                                   // append staff to track 
                                   symrec_t *m = getsym (d, "master");
                                   printf ("[debug] getsym master %p\n", m);
                                   symrec_t *right = m->value.master->track;

                                   if (right != NULL)
                                   {
                                   for (symrec_t * a = m->value.master->track; 1; a = a->next)
                                   if (a->next == NULL)
                                   {
                                   a->next = symrec_tcpy (s);
                                   printf ("[append staff] %s to head %p\n",
                                   a->value.staff.identifier,
                                   m->value.master->track);
                                   break;
                                   }
                                   }
                                   else
                                   m->value.master->track = symrec_tcpy (s);
                                   }
                                   while (1);
                                   rule_match = 1;
                                   } */
  else
    return 0;

  if (tok != TOK_SEMICOLON)
    UNEXPECTED (tok, d);

  tok = read_next_tok_cur_dats_t (d);

  return 0;
}

static int
parse_master ()
{
  /* insert one master */
  symrec_t *m = malloc (sizeof (symrec_t));
  m->type = TOK_MASTER;
  m->line = line_token_found;
  m->column = column_token_found;
  m->value.master = NULL;
  m->next = d->sym_table;
  d->sym_table = m;

  tok = read_next_tok_cur_dats_t (d);
  if (tok != TOK_LCURLY_BRACE)
    {
      UNEXPECTED (tok, d);
    }
  tok = read_next_tok_cur_dats_t (d);
  do
    {
      rule_match = 0;
      if (parse_track ())
        return 1;
    }
  while (rule_match);

  if (tok != TOK_RCURLY_BRACE)
    UNEXPECTED (tok, d);

  return 0;
}

static int
start ()
{

  switch (tok)
    {
    case TOK_STAFF:
      do
        {
          if (parse_staff ())
            return 1;
        }
      while (tok == TOK_STAFF);
      return local_errors;
    case TOK_MASTER:
      do
        {
          if (parse_master ())
            return 1;
        }
      while (tok == TOK_MASTER);
      return local_errors;
    case TOK_EOF:
      return local_errors;
    default:
      UNEXPECTED (tok, d);
    }

  return local_errors;
}

/* Returns 0 if success. Non-zero if failed. */
int
parse_cur_dats_t (dats_t * const t)
{
  local_errors = 0;
  d = t;

  while (1)
    {
      tok = read_next_tok_cur_dats_t (d);
      if (start ())
        {
          ERROR ("%d local errors generated\n", local_errors);
          global_errors += local_errors;
          local_errors = 0;
          free (tok_identifier);
          tok_identifier = NULL;
          return 1;
        }
      if (tok == TOK_EOF)
        break;
    }
  printf ("[\x1b[1;32m%s:%d @ %s\x1b[0m] %s: parsing successful\n",
          __FILE__, __LINE__, __func__, d->fname);

  return local_errors;
}
