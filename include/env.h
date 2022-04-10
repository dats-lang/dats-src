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

#ifndef ENV_H
#define ENV_H
#include <stdint.h>
#include <stdio.h>

enum token_t {
  TOK_IDENTIFIER,
  TOK_STRING,

  /* Data types */
  TOK_STAFF,
  // TOK_TRACK,
  TOK_SYNTH,
  TOK_FILTER,
  TOK_MAIN,
  TOK_PCM16,
  TOK_FLOAT,

  /* Macros */
  TOK_REPEAT,
  TOK_WRITE,
  TOK_READ,
  TOK_MIX,
  TOK_NOTE,
  TOK_N,
  TOK_R,

  /* env */
  TOK_BPM,
  TOK_ATTACK,
  TOK_DECAY,
  TOK_SUSTAIN,
  TOK_RELEASE,
  TOK_SEMITONE,
  TOK_OCTAVE,
  TOK_VOLUME,

  /* Symbols */
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_LCURLY_BRACE,
  TOK_RCURLY_BRACE,
  TOK_LBRACKET,
  TOK_RBRACKET,
  TOK_SEMICOLON,
  TOK_COMMA,
  TOK_DOT,
  TOK_DQUOTE, /*  double quote */
  TOK_SQUOTE, /* single qoute */
  TOK_UNDERSCORE,

  TOK_EQUAL,
  TOK_ADD,
  TOK_SUB,
  TOK_MUL,
  TOK_DIV,

  /* Misc */
  TOK_EOF,
  TOK_ERR,
  TOK_NULL
};
typedef enum token_t token_t;

enum music_symbol { SYM_REST, SYM_NOTE, SYM_BLOCK };
typedef enum music_symbol music_symbol;

typedef struct note_t note_t;
struct note_t {
  /* duration of the sound */
  uint32_t duration; // for staccato and staccatissimo

  /* alternative for frequency */
  uint8_t mnkey; // midi note number

  float frequency;
  float velocity;
  float attack;
  float decay;
  float sustain;
  float release;
  uint32_t volume;
  note_t *next; /* a dyad or a chord maybe? */
};

typedef struct bnr_t bnr_t;
typedef struct nr_t nr_t; /* list of notes and rests with properties */
struct nr_t {
  music_symbol type;
  union {
    struct {
      bnr_t *block;
    };

    struct {
      /* length of musical note/rest */
      uint32_t length;
      note_t *note; /* if type = SYM_NOTE */
    };
  };
  nr_t *next;
};

struct bnr_t {
  uint32_t nb_samples;
  uint8_t block_id, block_repeat;
  nr_t *block;
};

typedef struct synth_option_t synth_option_t;
typedef struct synth_option_t filter_option_t;
struct synth_option_t {
  char *option_name, is_strv;
  uint32_t line, column;
  union {
    float floatv;
    char *strv;
  };
};

enum pcm16_type_t { ID, MIX, FILTER, SYNTH };
typedef enum pcm16_type_t pcm16_type_t;

typedef struct pcm16_t pcm16_t;
struct pcm16_t {
  pcm16_type_t type;
  union {
    struct {
      char *id;
      size_t line, column;
    } ID;
    struct {
      uint32_t nb_pcm16;
      pcm16_t **pcm16; // an array
      size_t line, column;
    } MIX;
    struct {
      char *filter_name;
      pcm16_t *pcm16_arg; // a linked list
      uint32_t filter_line, filter_column;
      uint32_t pcm16_line, pcm16_column;
      uint16_t nb_options;
      filter_option_t *options;
    } FILTER;
    struct {
      char *synth_name, *staff_name;
      uint32_t staff_line, staff_column;
      uint32_t synth_line, synth_column;
      uint16_t nb_options;
      synth_option_t *options;
    } SYNTH;
  };
  int16_t *pcm;
  uint32_t nb_samples;
  float gain;

/*  A play_end is suppose to mark the end of a playing.
 *  This is needed because the rest of it might just be
 *  reverberation. Like for example: If you stop playing,
 *  the instrument continues to attenuate. It does not
 *  immediately stop.
 *
 *  play_end is always less than or equal to nb_samples;
 *   play_end <= nb_samples
 */
  uint32_t play_end;

  pcm16_t *next;
};

typedef struct symrec_t symrec_t;
struct symrec_t {
  token_t type;
  size_t line, column;
  union {
    struct {
      char *identifier;
      nr_t *nr;
      bnr_t *bnr;
      uint32_t nb_samples; // with the repeat
    } staff; /* staff variables */

    struct {
      char *out_file;
      pcm16_t *pcm;
 //     uint32_t nb_samples;
    } write;
    /* struct
     {
       char *identifier;
       float val;
     } env;*/			/* environment variables */

    struct {
      char *identifier;
      uint32_t nb_samples;
      pcm16_t *pcm;
    } pcm16;
  } value;

  symrec_t *next;
};

typedef struct dats_t dats_t;
struct dats_t {
  /* `fp` contains the processed file */
  /* `initial` contains the raw unprocessed file,
   *   and will be processed by the processor.
   */
  FILE *fp, *initial;
  char *fname;
  char scan_line[500];
  uint32_t line, column;
  symrec_t *sym_table;

  dats_t *next;
};

#endif
