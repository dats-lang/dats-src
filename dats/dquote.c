/* dquote.c - Print inspiring life quotes everytime your
 * software crashes.
 *
 * The author disclaims copyright to this source code.
 * In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *
 *    May you find forgiveness for yourself and forgive others.
 *
 *    May you share freely, never taking more than you give.
 *
 **/

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

const uint32_t nb_quotes = 346;
static uint32_t i = 0;

typedef struct quote_t quote_t;
struct quote_t {
  char *quote;
};

static const quote_t quotes[] = {
  #include "quotes.txt"
};

void dats_print_quote(void){
  srand(time(NULL));
  i = (uint32_t)rand() % nb_quotes;
  write(2, quotes[i].quote, strlen(quotes[i].quote));
}
