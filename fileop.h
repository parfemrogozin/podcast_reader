#ifndef my_header_stuff
#define my_header_stuff
  #define ITEMSIZE 81
  #define SCREENSIZE 1920
  #define URLMAX 2048
  #define QUEUENAME "/podcast_reader_requests"
#endif

enum names_of_pahs
{
  URL_LIST,
  MUSIC_DIRECTORY,
  LOCALE_PATH,
  SAVE_TEMPLATE
};
#include "id3op.h"
/* TODO: Add status of download */
struct Download_data
{
  char * url;
  struct id3v1 id3;
};

int set_paths(void);
void add_url(void);
int count_lines(FILE *fp);
char * create_feed_list(unsigned int *record_count);
int download_file(char * url, char * filename);
void * start_downloader();
