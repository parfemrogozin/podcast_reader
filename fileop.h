#ifndef my_header_stuff
#define my_header_stuff
  #define ITEMSIZE 81
  #define SCREENSIZE 1920
  #define URLMAX 2048
  #define BASENAMESIZE 17
  #define SUFFIXSIZE 5
#endif

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
  char filename[BASENAMESIZE + SUFFIXSIZE];
  char directory[ITEMSIZE];
  char * url;
};

int set_paths(void);
void add_url(void);
int count_lines(FILE *fp);
char * create_feed_list(int *lines);
int download_file(char * url, char * filename);
void * start_downloader();
