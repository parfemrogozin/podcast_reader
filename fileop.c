#include <unistd.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <ncurses.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <mqueue.h>

#include "fileop.h"

#include <locale.h>
#include <libintl.h>

#ifndef translation
#define translation
  #define _(STRING) gettext(STRING)
#endif

char READER_PATHS[4][80];

int set_paths(void)
{
  if (getenv("XDG_CONFIG_HOME"))
  {
    strcpy(READER_PATHS[URL_LIST], getenv("XDG_CONFIG_HOME"));
  }
  else
  {
    strcpy(READER_PATHS[URL_LIST], getenv("HOME"));
    strcat(READER_PATHS[URL_LIST], "/.config");
  }
  strcat(READER_PATHS[URL_LIST], "/rss_feed_list.txt");

  if( access( "/usr/bin/xdg-user-dir", X_OK ) == 0 )
  {
    FILE * program_output;
    program_output = popen("xdg-user-dir MUSIC", "r");
    fscanf(program_output, "%s\n", READER_PATHS[MUSIC_DIRECTORY]);
    pclose(program_output);
  }
  else
  {
    strcpy(READER_PATHS[MUSIC_DIRECTORY], getenv("HOME"));
  }
  strcat(READER_PATHS[MUSIC_DIRECTORY], "/Podcasts");
  mkdir(READER_PATHS[MUSIC_DIRECTORY], 0700);

  if (getenv("XDG_DATA_HOME"))
  {
  strcpy(READER_PATHS[LOCALE_PATH], getenv("XDG_DATA_HOME"));
  }
  else
  {
    strcpy(READER_PATHS[LOCALE_PATH], getenv("HOME"));
    strcat(READER_PATHS[LOCALE_PATH], "/.local/share");
  }
  strcat(READER_PATHS[LOCALE_PATH], "/locale");
  mkdir(READER_PATHS[LOCALE_PATH], 0700);

  strcpy(READER_PATHS[SAVE_TEMPLATE], "/tmp/rss%d.xml");
  return 0;
}

void add_url(void)
{
  char rss_url[80];
  FILE *url_list;
  url_list = fopen(READER_PATHS[URL_LIST], "a");

  echo();
  mvprintw(LINES-1, 0,"%s", _("Enter RSS feed address: "));
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
    printf(_("Could not open file %s"), READER_PATHS[URL_LIST]);
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


/* REWORK

  sprintf(split_command, "mp3splt -Q -t 10.00 -o @f/@n2 %s", ddata->filename);
  system(split_command);
  unlink(ddata->filename);

  return NULL;
}
*/

void * start_downloader()
{
  int run = 1;
  unsigned int msg_prio;
  struct Download_data  * request;
  char buffer[sizeof( struct Download_data)];
  mqd_t queue = mq_open (QUEUENAME, O_RDONLY);


  while (run)
  {
    mq_receive(queue, buffer, sizeof(struct Download_data), &msg_prio);
    if (msg_prio == 1)
    {
    request = (struct Download_data *) buffer;


    chdir(READER_PATHS[MUSIC_DIRECTORY]);
    mkdir(request->directory, 0700);
    chdir(request->directory);
    strcat(request->filename, ".mp3");

    move(LINES-1,0);
    clrtoeol();
      printw("%s: %s", _("Downloading"), request->filename);
    refresh();

    download_file(request->url, request->filename);

    move(LINES-1,0);
    clrtoeol();
    refresh();
    }
    else
    {
      run = 0;
    }
  }
  mq_close (queue);
  return NULL;
}


char * create_feed_list(int *lines)
{
  int cursor_pos = 11;
  FILE *url_list;
  url_list = fopen(READER_PATHS[URL_LIST], "r");
  *lines = count_lines(url_list);
  char feed_address[ITEMSIZE];
  char * file_names = malloc(*lines * ITEMSIZE);
  for (int i = 0; i < *lines; ++i)
  {
    fgets(feed_address,ITEMSIZE,url_list);
    strtok(feed_address, "\n");
    sprintf (file_names + ITEMSIZE * i, READER_PATHS[SAVE_TEMPLATE], i);
    download_file(feed_address, file_names  + ITEMSIZE * i);
    mvprintw(LINES-1, cursor_pos + i, "%s", ".");
    refresh();
  }
  fclose(url_list);
  return file_names;
}
