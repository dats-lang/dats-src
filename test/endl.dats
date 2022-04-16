/* Endless love by Neal K */

staff m1 {
  bpm = 95;
  // octave = 1; // raise the note one octave
  semitone = 1;
  n 4., e4;
  n 8, c5;
  n 4., a4;
  n 8, e5;

  n 4, d5;
  n 8, c5;
  n 8, d5;
  n 4, e5;
  n 8, c5;
  n 8, d5;

  n 4., c5;
  n 8, g5;
  n 4, c5;
  n 8, a5;
  n 8, g5;

  n 2, d5;
  n 8, e5;
  n 8, f5;
  n 8, e5;
  n 8, d5;

  n 4, c5;
  n 4, g5;
  n 8, c5;
  n 8, g5;
  n 8, c6;
  n 8, b5;

  n 4, c5;
  n 8, d5;
  n 24, d5;
  n 24, c5;
  n 24, d5;
  n 4, e5;
  n 8, a4;
  n 8, b4;

  n 4., c5;
  n 4., e5;
  n 4, c5;

  n 4., b4;
  n 16, a4;
  n 16, b4;
  n 2, c5; 
}

staff m2 {
  bpm = 95;
  octave = 1;
  semitone = 1;
  n 8, a3;
  n 8, e4;
  n 4, c5;
  n 8, f3;
  n 8, c4;
  n 4, a4;

  n 8, g3;
  n 8, d4;
  n 4, b4;
  n 8, c4;
  n 8, g4;
  n 8, e5;
  n 8, d5;

  n 8, a3;
  n 8, e4;
  n 4, c5;
  n 8, f3;
  n 8, c4;
  n 4, a4;

  n 8, g3;
  n 8, d4;
  n 4, b4;
  n 8, c4;
  n 8, g4;
  n 8, e5;
  n 8, d5;
  
  n 8, a3;
  n 8, e4;
  n 4, c5;
  n 8, f3;
  n 8, c4;
  n 4, a4;

  n 8, g3;
  n 8, d4;
  n 4, b4;
  n 8, c4;
  n 8, g4;
  n 8, e5;
  n 8, d5;

  n 8, a3;
  n 8, e4;
  n 4, c5;
  n 8, f3;
  n 8, c4;
  n 4, a4;

  n 8, g3;
  n 8, d4;
  n 4, b4;
  n 8, c4;
  n 8, g4;
  n 4, c5;
}

staff m3 {
  bpm = 95;
  octave = 1;
  semitone = 1;

  n 2, c5;
  n 2, a4;

  n 2, b4;
  n 2, e5;

  n 2, c5;
  n 2, a4;

  n 2, b4;
  n 2, e5;

  n 2, c5;
  n 2, a4;

  n 2, b4;
  n 2, e5;

  n 2, c5;
  n 2, a4;

  n 2, b4;
  n 2, e5;


}
main {
  track tr1 = synth.sf2(m1)[preset=9];
  track tr2 = synth.sf2(m2)[preset=10];
  track tr3 = synth.sin(m3);
  write("t.wav", mix((tr1), (tr2)));
}
