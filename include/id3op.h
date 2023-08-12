#ifndef ID3OP_H_INCLUDED
#define ID3OP_INCLUDED
struct id3v1
{
  char title[30];
  char artist[30];
  char album[30];
  char year[4];
  char comment[30];
  unsigned char genre;
};


int remove_id3tags(char * mp3filename);
int add_id3tags(char * mp3filename, struct id3v1 tags);
#endif /* ID3OP_H_INCLUDED */