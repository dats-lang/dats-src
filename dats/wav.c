/* The author disclaims copyright to this source code, and
 * puts it under public domain.
 */

#include "wav.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int wav_write_header(const struct WAV_info *const wav) {
  assert(wav->fp != NULL);

  fwrite("RIFF", sizeof(char), 4, wav->fp);
  uint32_t Subchunk2Size =
      wav->NumSamples * wav->NumChannels * (wav->BitsPerSample / 8);
  uint32_t ChunkSize = 20 + wav->Subchunk1Size + Subchunk2Size;
  uint32_t ByteRate =
      wav->SampleRate * wav->NumChannels * (wav->BitsPerSample / 8);

  fwrite(&ChunkSize, sizeof(uint32_t), 1, wav->fp);
  fwrite("WAVE", sizeof(char), 4, wav->fp);
  fwrite("fmt ", sizeof(char), 4, wav->fp);
  fwrite(&wav->Subchunk1Size, sizeof(uint32_t), 1, wav->fp);
  fwrite(&wav->AudioFormat, sizeof(int16_t), 1, wav->fp);
  fwrite(&wav->NumChannels, sizeof(int16_t), 1, wav->fp);
  fwrite(&wav->SampleRate, sizeof(uint32_t), 1, wav->fp);
  fwrite(&ByteRate, sizeof(uint32_t), 1, wav->fp);
  fwrite(&(int){wav->NumChannels * (wav->BitsPerSample / 8)}, sizeof(int16_t),
         1, wav->fp);
  fwrite(&wav->BitsPerSample, sizeof(uint16_t), 1, wav->fp);
  fwrite("data", sizeof(char), 4, wav->fp);
  fwrite(&Subchunk2Size, sizeof(uint32_t), 1, wav->fp);

  return 0;
}
