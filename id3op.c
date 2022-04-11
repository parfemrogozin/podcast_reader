#include <stdio.h>


size_t decode7bit(unsigned char four_bytes[])
{
  unsigned int four_ints[4];
  unsigned int shift = 21;

  for (int i=0; i < 4; i++)
  {
    four_ints[i] = four_bytes[i];
    four_ints[i] = four_ints[i] << shift;
    printf("Posun o %d\n", shift);
    shift -= 7;
  }

  return four_ints[0] + four_ints[1] + four_ints[2] + four_ints[3];
}


int main(int argc, char **argv)
{
  FILE *mp3file;
  FILE *cutfile;
  size_t block_size;
  char read_buffer[BUFSIZ];
  unsigned char four_bytes[4];

  mp3file = fopen(argv[1], "r");
  fseek(mp3file, 6, SEEK_SET);
  fread(four_bytes, 1, 4, mp3file);
  block_size = decode7bit(four_bytes);
  fseek(mp3file, block_size, SEEK_CUR);
  cutfile = fopen("out.mp3", "w");
  while ( feof(mp3file) == 0)
  {
    fread(read_buffer, BUFSIZ, 1, mp3file);
    fwrite(read_buffer, BUFSIZ, 1, cutfile);
  }
  fclose(mp3file);
  fclose(cutfile);

  return 0;
}

