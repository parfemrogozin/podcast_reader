#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "include/id3op.h"


static size_t decode7bit(unsigned char four_bytes[])
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


static int remove_id3v2(char * filename)
{
  int ret_code;
  char first_bytes[3];
  FILE *mp3file;
  mp3file = fopen(filename, "r");

  fread(first_bytes, 1, 3, mp3file);

  if (first_bytes[0] == 'I' && first_bytes[1] == 'D' && first_bytes[2] == '3')
  {
    FILE *cutfile;
    size_t block_size;
    char * temp_filename = "out.mp3";
    char read_buffer[BUFSIZ];
    unsigned char four_bytes[4];

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
    fclose(cutfile);
    fclose(mp3file);
    rename(temp_filename, filename);
    ret_code = 0;
  }
  else
  {
    fclose(mp3file);
    ret_code = -1;
  }
  return ret_code;
}

static off_t get_offset_id3v1(char * mp3filename)
{
  int ret_value;
  char tag_buffer[4];
  off_t position;
  FILE *mp3file;
  mp3file = fopen(mp3filename, "r");
  memset(tag_buffer, 0, sizeof(tag_buffer));
  fseek(mp3file, -128, SEEK_END);
  position = ftello(mp3file);
  fread(tag_buffer, 1, 3, mp3file);
  if (tag_buffer[0] == 'T' && tag_buffer[1] == 'A' && tag_buffer[2] == 'G')
  {
    ret_value = position;
  }
  else
  {
    ret_value = 0;
  }
  return ret_value;
}

static int remove_id3v1(char * mp3filename)
{
  int ret_code;
  off_t offset;
  offset = get_offset_id3v1(mp3filename);
  if (offset > 0)
  {
    truncate(mp3filename, offset);
    ret_code = 0;
  }
  else
  {
    ret_code = -1;
  }
  return ret_code;
}

int remove_id3tags(char * mp3filename)
{
  remove_id3v2(mp3filename);
  remove_id3v1(mp3filename);
  return 0;
}

int add_id3tags(char * mp3filename, const char * feed, const char * episode)
{
  const char * tag = "TAG";
  /* former struct -> */
  char title[30] = {'\0'};
  char artist[30] = {'\0'};
  char album[30] = {'\0'};
  char year[4] = {'\0'};
  char comment[30] = {'\0'};
  unsigned char genre = 186;
  /* <- */

  strncpy(artist, feed, 30);
  strncpy(album, episode, 30);

  FILE *mp3file;
  mp3file = fopen(mp3filename, "a");
    fwrite(tag, 3, 1, mp3file);
    fwrite(title, sizeof(title), 1, mp3file);
    fwrite(artist, sizeof(artist), 1, mp3file);
    fwrite(album, sizeof(album), 1, mp3file);
    fwrite(year, sizeof(year), 1, mp3file);
    fwrite(comment, sizeof(comment), 1, mp3file);
    fwrite(&genre, sizeof(genre), 1, mp3file);
  fclose(mp3file);

  return 0;
}