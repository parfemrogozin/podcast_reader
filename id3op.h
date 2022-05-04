#ifndef id3define
#define id3define
struct id3v1
{
  char title[30];
  char artist[30];
  char album[30];
  char year[4];
  char comment[30];
  unsigned char genre;
};
#endif

int remove_id3tags(char * mp3filename);
int add_id3tags(char * mp3filename, struct id3v1 tags);
