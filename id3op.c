#include <stdio.h>


size_t decode7bit(unsigned char four_bytes[])
{
  unsigned int four_ints[4];
  unsigned int shift = 21;

  for (int i=0; i < 4; i++)
  {
    four_ints[i] = four_bytes[i];
    four_ints[i] = four_ints[i] << shift;
    shift -= 7;
  }

  return four_ints[0] + four_ints[1] + four_ints[2] + four_ints[3];
}


int remove_id3v2(char * filename)
{
  FILE *mp3file;
  FILE *cutfile;
  size_t block_size;
  char * temp_filename = "out.mp3";
  char read_buffer[BUFSIZ];
  char first_bytes[3];
  unsigned char four_bytes[4];

  mp3file = fopen(filename, "r");
  fread(first_bytes, 1, 3, mp3file);
  if (first_bytes[0] == 'I' && first_bytes[1] == 'D' && first_bytes[2] == '3')
  {
    fseek(mp3file, 3, SEEK_CUR);
    fread(four_bytes, 1, 4, mp3file);
    block_size = decode7bit(four_bytes);
    fseek(mp3file, block_size, SEEK_CUR);
    cutfile = fopen(temp_filename, "w");
    while ( feof(mp3file) == 0)
    {
      fread(read_buffer, BUFSIZ, 1, mp3file);
      fwrite(read_buffer, BUFSIZ, 1, cutfile);
    }
    fclose(mp3file);
    fclose(cutfile);
    rename(temp_filename, filename);
    return 0;
  }
  else
  {
    fprintf(stderr, "ID3v2 tag header not found. \n");
    return 1;
  }
}

int main(int argc, char **argv)
{
  remove_id3v2(argv[1]);
  return 0;
}

