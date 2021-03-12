#include <stdlib.h>
#include <curl/curl.h>
#include <ncurses.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fileop.h"

void add_url(void)
{
  char rss_url[80];
  FILE *url_list;
  url_list = fopen(URL_LIST, "a");

  echo();
  mvprintw(LINES-1, 0,"%s", "Vlož adresu: ");
  getstr(rss_url);
  noecho();

  fputs(rss_url, url_list);
  fputc('\n', url_list);
  fclose(url_list);
}

int count_lines(FILE *fp)
{
  int line_count;
  int c;

  if (fp == NULL)
  {
    printf("Could not open file %s", URL_LIST);
    return -1;
  }

  for (c = getc(fp); c != EOF; c = getc(fp))
  {
    if (c == '\n')
    {
      line_count++;
    }
  }
  rewind(fp);
  return line_count;
}



int download_file(char * url, char * filename)
{
  CURL * downloader;
  FILE * tmp_file = fopen(filename, "w");
  downloader = curl_easy_init();
  curl_easy_setopt(downloader, CURLOPT_URL, url);
  curl_easy_setopt(downloader, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(downloader, CURLOPT_WRITEDATA, tmp_file);
  curl_easy_setopt(downloader, CURLOPT_NOPROGRESS, 1);

  curl_easy_perform(downloader);
  curl_easy_cleanup(downloader);
  fclose(tmp_file);
  return 0;
}

void * threaded_download(void * download_struct_ptr)
{
  struct Download_data * ddata = (struct Download_data *) download_struct_ptr;
  size_t dir_name_len = strlen(ddata->directory);
  char * output_name = malloc(dir_name_len + 1 + BASENAMESIZE + SUFFIXSIZE);
  mkdir(ddata->directory, 0700);
  sprintf (output_name,"%s/%s", ddata->directory, ddata->filename);
  download_file(ddata->url, output_name);
  free(output_name);
  return NULL;
}

char * create_feed_list(int *lines)
{
  int cursor_pos = 11;
  FILE *url_list;
  url_list = fopen(URL_LIST, "r");
  *lines = count_lines(url_list);
  char feed_address[ITEMSIZE];
  char * file_names = malloc(*lines * ITEMSIZE);
  for (int i = 0; i < *lines; ++i)
  {
    fgets(feed_address,ITEMSIZE,url_list);
    strtok(feed_address, "\n");
    sprintf (file_names + ITEMSIZE * i,"rss%d.xml", i);
    download_file(feed_address, file_names  + ITEMSIZE * i);
    mvprintw(LINES-1, cursor_pos + i, "%s", ".");
    refresh();
  }
  fclose(url_list);
  return file_names;
}
