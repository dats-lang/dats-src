set -ex

CC="gcc"
AR="ar"
CFLAGS="-Wall -Wextra -O2 -I.. -I../include"

# COMPILES SNDFILTER
pushd sndfilter
OBJ=(mem snd wav biquad compressor reverb)
for i in ${!OBJ[@]}; do
	$CC -c ${OBJ[$i]}.c -fPIC $CFLAGS -o ${OBJ[$i]}.o
done;
$AR rcs libsndfiler.a ${OBJ[*]/%/.o}
popd

# COMPILES LIBDFILTER
pushd libdfilter
OBJ=(allfilter f_reverb)
for i in ${!OBJ[@]}; do
	$CC -c ${OBJ[$i]}.c -fPIC $CFLAGS -o ${OBJ[$i]}.o
done;
$CC ${OBJ[*]/%/.o} -fPIC -shared $CFLAGS -lm -L../sndfilter -lsndfilter -o libdfilter.so
popd

# COMPILES LIBDSYNTH
pushd libdsynth
OBJ=(allsynth utils s_kpa s_sin s_square s_sf2)
for i in ${!OBJ[@]}; do
	$CC -c ${OBJ[$i]}.c -fPIC $CFLAGS -o ${OBJ[$i]}.o
done;
$CC ${OBJ[*]/%/.o} -fPIC -shared $CFLAGS -lm -lfluidsynth -o libdsynth.so
popd

# COMPILES DATS
pushd dats
OBJ=(dats parser scanner wav semantic utils dquote genpcm execute mixer)
for i in ${!OBJ[@]}; do
	$CC -c ${OBJ[$i]}.c $CFLAGS -o ${OBJ[$i]}.o
done;
$CC ${OBJ[*]/%/.o} $CFLAGS -L../libdfilter -L../libdsynth -ldfilter -ldsynth -lm -o dats
popd

# prevents rpath error
cp libdsynth/libdsynth.so libdfilter/libdfilter.so dats
