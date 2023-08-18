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



int set_paths(void);
int get_feed_list(void);
void add_url(void);
void sanitize(char in_place_string[160]);
int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
#endif /* FILEOP_H_INCLUDED */