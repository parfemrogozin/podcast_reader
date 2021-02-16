#ifndef my_header_stuff
#define my_header_stuff
  #define URL_LIST "./ulr_list.txt"
  #define ITEMSIZE 80
#endif

void add_url(void);
int count_lines(FILE *fp);
char * create_feed_list(int *lines);
int download_feed_file(char * url, char * filename);



