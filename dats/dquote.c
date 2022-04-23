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

#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
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
  write(2, quotes[i].quote, strlen(quotes[i].quote));
}

void dats_exit_handler(int sig, siginfo_t *info, void *ucontext) {
  write(2, "Segmentation fault\n", 19);
  dats_print_quote();
  _exit(sig);
}

#ifndef _WIN32
void register_deadquote(void) {
  srand(time(NULL));
  i = (uint32_t)rand() % nb_quotes;
  struct sigaction sa = {0};
  sa.sa_sigaction = &dats_exit_handler;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
}
#endif
