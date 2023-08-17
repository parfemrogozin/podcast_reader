#include "id3op.h"
#ifndef FILEOP_H_INCLUDED
#define FILEOP_INCLUDED
enum names_of_pahs
{
  URL_LIST,
  MUSIC_DIRECTORY,
  LOCALE_PATH,
  CACHE_PATH,
  TMP_FILE,
  FEED_TEMPLATE
};

/* TODO: Add status of download */
struct Download_data
{
  char * url;
  struct id3v1 id3;
  char dest_dir[80+1+30+1];
  char dest_file[30+1+3+1];
};

int set_paths(void);
int get_feed_list(void);
void add_url(void);
void set_download_destination(struct Download_data * download_data);
int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
#endif /* FILEOP_H_INCLUDED */