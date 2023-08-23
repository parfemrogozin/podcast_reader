#ifndef ID3OP_H_INCLUDED
#define ID3OP_INCLUDED

int remove_id3tags(char * mp3filename);
int add_id3tags(char * mp3filename, const char * feed, const char * episode);
#endif /* ID3OP_H_INCLUDED */