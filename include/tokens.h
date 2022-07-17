#ifndef TOKENS_H
#define TOKENS_H

enum token_t {
  TOK_IDENTIFIER,
  TOK_STRING,

  /* Data types */
  TOK_STAFF,
  // TOK_TRACK,
  TOK_ATRACK,
  TOK_SYNTH,
  TOK_FILTER,
  TOK_MAIN,
  TOK_TRACK,
  TOK_FLOAT,

  /* Type specifier*/
  TOK_MONO,
  TOK_STEREO,

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

#endif /* TOKENS_H */
