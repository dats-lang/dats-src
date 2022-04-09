rem Copy this file to ../

set CC="gcc"
set CFLAGS="-Wall -Wextra -O2  -I.. -I../include"
set LDFLAGS="-L../libdsynth -ldsynth -L../libdfilter -ldfilter -lm"

rem Buils sndfilter

cd sndfilter


cd dats
for %%i in (dats.c scanner.c parser.c dquote.c semantic.c
  wav.c utils.c) do %%CC %%CFLAGS
%%CC %%CFLAGS dats.o scanner.o parser.o dquote.o semantic.o \
  wav.o utils.o -o dats
cd ../libdfilter
