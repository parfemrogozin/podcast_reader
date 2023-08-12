#include "id3op.h"
#ifndef FILEOP_H_INCLUDED
#define FILEOP_INCLUDED
enum names_of_pahs
{
  URL_LIST,
  MUSIC_DIRECTORY,
  LOCALE_PATH,
  SAVE_TEMPLATE
};

/* TODO: Add status of download */
struct Download_data
{
  char * url;
  struct id3v1 id3;
};

int set_paths(void);
void add_url(void);
char * create_feed_list(unsigned int *record_count);
int download_file(char * url, char * filename);
void * start_downloader();
#endif /* FILEOP_H_INCLUDED */