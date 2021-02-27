#ifndef my_header_stuff
#define my_header_stuff
  #define URL_LIST "./ulr_list.txt"
  #define ITEMSIZE 81
#endif

struct Download_data
{
  char filename[22];
  char directory[ITEMSIZE];
  char * url;
};

void add_url(void);
int count_lines(FILE *fp);
char * create_feed_list(int *lines);
int download_file(char * url, char * filename);

void * threaded_download(void * download_struct_ptr);



