staff chor {
  volume = 1100;
  n 12, d4;
  n 12, f#4;
  n 12, a4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;

  n 12, b3;
  n 12, d4;
  n 12, g4;
  n 12, b3;
  n 12, d4;
  n 12, g4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;
}

staff m1 {
  bpm = 120;
  octave = 1; // raise an octave
  semitone = 0; // raise a 0 semitone

  repeat 1 {
    n 8., d5.;
    n 16, e5;
    n 8., f#5.;
    n 16, g5;
    n 4, a5.;
    n 4, d5.;
  
    n 4, b5.;
    n 4, d6.;
    n 4, a5.;
    n 4, f#5.;
  
    n 8., g5.;
    n 16, f#5;
    n 8., g5.;
    n 16, a5;
    n 8., f#5.;
    n 16, e5;
    n 8., d5.;
    n 16, a4;
  
    n 8., e5.;
    n 16, d#5;
    n 8., e5.;
    n 16, f#5;
    n 4, e5;
    n 4, a4;
  }
  n 8., b4.;
  n 16, a#4;
  n 8., b4.;
  n 16, c#5;
  n 4, d5;
  n 8., a4.;
  n 16, f#5;

  n 8., f#5;
  n 32, g5;
  n 32, f#5;
  n 8., e5.;
  n 16, d5;
  n 4, e5;
  n 8., a4.;
  n 16, a4;
// rmm
  n 8., d5.;
  n 16, e5;
  n 8., f#5.;
  n 16, g5;
  n 4, a5.;
  n 4, d5.;

  n 4, b5.;
  n 4, d6.;
  n 4, a5.;
  n 4, f#5.;

  n 8., g5.;
  n 16, f#5;
  n 8., e5.;
  n 16, a4;
  n 8., d5.;
  n 16, e5;
  n 8., f#5.;
  n 16, d5;

  n 12, c#5;
  n 12, a4;
  n 12, e4;
  n 12, a4;
  n 12, c#5;
  n 12, e5;
  n 4, d5;
  n 4, a4 f#5 d6;
}

staff m2 {
// part 1
  octave = 0;
  volume = 1100;
  repeat 1 {
    chor;
  
    n 12, b3;
    n 12, d4;
    n 12, g4;
    n 12, b3;
    n 12, d4;
    n 12, g4;
    n 12, d4;
    n 12, f#4;
    n 12, a4;
    n 12, d4;
    n 12, f#4;
    n 12, a4;
  
    n 12, e4;
    n 12, g4;
    n 12, b4;
    n 12, e4;
    n 12, g4;
    n 12, b4;
    n 12, a4;
    n 12, c#5;
    n 12, e5;
    n 12, c#5;
    n 12, a4;
    n 12, g4;
  }
  
  n 12, g3;
  n 12, b3;
  n 12, d4;
  n 12, g3;
  n 12, b3;
  n 12, d4;
  n 12, a3;
  n 12, d4;
  n 12, f#4;
  n 12, a3;
  n 12, d4;
  n 12, f#4;

  n 12, g#3;
  n 12, b3;
  n 12, e4;
  n 12, g#3;
  n 12, b3;
  n 12, e4;
  n 12, a4;
  n 12, c#5;
  n 12, e5;
  n 12, a3;
  n 12, a4;
  n 12, a5;

  chor;

  n 12, c#4;
  n 12, e4;
  n 12, g4;
  n 12, a4;
  n 12, e4;
  n 12, c#4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;

  n 12, c#4;
  n 12, e4;
  n 12, a4 e5;
  n 12, g4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;
  n 12, f#4;
  n 12, a4;
  n 12, d4;
  n 12, f#4;
  n 12, a4;
}

staff m3 {
  octave = -1;
  repeat 1 {
    // part 1
    n 4, d3;
    n 4, f#3;
    n 4, a3;
    n 4, d4;
  
    n 4, g3;
    n 4, b3;
    n 4, d4;
    n 4, f#3;
  
    n 4, g3;
    n 4, b3;
    n 4, d3;
    n 4, f#3;
  
    n 4, e3;
    n 4, g#3;
    n 4, a3;
    n 4, c3;
  }
}

main {
  track tr1 = mix((0.5 filter.reverb(synth.sin(m1)[
    vibrato_frequency=8, vibrato_magnitude=3])),
    (mix((0.4 synth.square(m2)),(0.4 synth.sin(m2)))));
  write("t.wav", tr1);

}
