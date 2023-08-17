#include <unistd.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <ncurses.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "include/pr_const.h"
#include "include/fileop.h"
#include "include/strop.h"


#include <locale.h>
#include <libintl.h>

#ifndef translation
#define translation
  #define _(STRING) gettext(STRING)
#endif

char READER_PATHS[6][80] = {0};


int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
  size_t * gui_pos = (size_t *)clientp;
  move(gui_pos[0], gui_pos[1] + 2);
  clrtoeol();
  mvprintw(gui_pos[0], gui_pos[1] + 2, "%d", dlnow);
  refresh();
  return 0;
}

void set_download_destination(struct Download_data * download_data)
{
  strcpy(download_data->dest_dir, READER_PATHS[MUSIC_DIRECTORY]);
  chdir(download_data->dest_dir);

  char dir_artist[30+1] = {0};
  strncpy(dir_artist, download_data->id3.artist, 30);
  sanitize_filename(dir_artist);
  mkdir(dir_artist, 0700);
  strcat(download_data->dest_dir, "/");
  strcat(download_data->dest_dir, dir_artist);
  chdir(download_data->dest_dir);

  char ord_num = 1;
  strncpy(download_data->dest_file, download_data->id3.album, 30);
  sanitize_filename(download_data->dest_file);

  while ( access( download_data->dest_file, F_OK ) == 0 )
  {
     ord_num++;
     download_data->dest_file[strlen(download_data->dest_file)-1] = '0' + ord_num;
  }
  strcat(download_data->dest_file, ".mp3");
}


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
  strcat(READER_PATHS[URL_LIST], "/podcast_reader");
  mkdir(READER_PATHS[URL_LIST], 0700);
  strcat(READER_PATHS[URL_LIST], "/rss_feed_list.txt"); /* 1 */

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
  strcat(READER_PATHS[MUSIC_DIRECTORY], "/Podcasts");  /* 2 */
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
  strcat(READER_PATHS[LOCALE_PATH], "/locale"); /* 3 */
  mkdir(READER_PATHS[LOCALE_PATH], 0700);

  if (getenv("XDG_CACHE_HOME"))
  {
    strcpy(READER_PATHS[CACHE_PATH], getenv("XDG_CACHE_HOME"));
  }
  else
  {
    strcpy(READER_PATHS[CACHE_PATH], getenv("HOME"));
    strcat(READER_PATHS[CACHE_PATH], "/.cache");
  }
  strcat(READER_PATHS[CACHE_PATH], "/podcast_reader"); /* 4 */
  mkdir(READER_PATHS[CACHE_PATH], 0700);

  strcpy(READER_PATHS[TMP_FILE], READER_PATHS[CACHE_PATH]);
  strcat(READER_PATHS[TMP_FILE], "/tmp.xml"); /* 5 */

  strcpy(READER_PATHS[FEED_TEMPLATE], READER_PATHS[CACHE_PATH]);
  strcat(READER_PATHS[FEED_TEMPLATE], "/rss%02d.xml"); /* 6 */

  return 0;
}

void add_url(void)
{
  char rss_url[URLMAX] = {0};
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

/*void * start_downloader(void)
{
  int run = 1;
  int ord_num;
  unsigned int msg_prio;
  struct Download_data  * request;
  char buffer[sizeof( struct Download_data)];
  char split_command[240] = {0};
  mqd_t queue = mq_open (QUEUENAME, O_RDONLY);

  while (run)
  {
    mq_receive(queue, buffer, sizeof(struct Download_data), &msg_prio);
    if (msg_prio == 1)
    {
    request = (struct Download_data *) buffer;


    move(LINES-1,0);
    clrtoeol();
      printw("%s: %s", _("Downloading"), filename);
    refresh();

    download_file(request->url, filename);
    free(request->url);
    remove_id3tags(filename);
    add_id3tags(filename, request->id3);
    sprintf(split_command, "mp3splt -Q -t 10.00 -o @f/@n2 -g r%%[@o,@N=1,@t=#t@N] %s", filename);
    system(split_command);
    unlink(filename);

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
}*/


int get_feed_list(void)
{
  int ret = 0;
  FILE *url_list;
  char address[URLMAX];
  if( access(READER_PATHS[URL_LIST], R_OK ) == 0 )
  {
    size_t gui_pos[2];

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, gui_pos);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_TIMECONDITION, (long)CURL_TIMECOND_IFMODSINCE);

    url_list = fopen(READER_PATHS[URL_LIST], "r");
    unsigned int i = 0;
    while ( fgets(address, URLMAX, url_list) != NULL )
    {
      char feed_file[80];
      sprintf(feed_file, READER_PATHS[FEED_TEMPLATE], i);
      long last_update = 0;
      struct stat attribute;
      int found = stat(feed_file, &attribute);
      if (0 ==  found)
      {
        last_update = (long)attribute.st_mtime;
      }
      FILE * tmp_file = fopen(READER_PATHS[TMP_FILE], "w");
      address[strcspn(address, "\n")] = 0;
      gui_pos[0] = i;
      gui_pos[1] = strlen(address);
      curl_easy_setopt(curl, CURLOPT_TIMEVALUE, last_update);
      curl_easy_setopt(curl, CURLOPT_URL, address);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, tmp_file);


      mvprintw(gui_pos[0], 0, "%s: ", address);
      CURLcode res = curl_easy_perform(curl);

      fclose(tmp_file);
      long unmet;
      curl_easy_getinfo(curl, CURLINFO_CONDITION_UNMET, &unmet);
      if (1L != unmet)
      {
        rename(READER_PATHS[TMP_FILE], feed_file);
      }
      else
      {
        mvprintw(gui_pos[0], gui_pos[1] + 2, "%s", _("NO CHANGE"));
        refresh();
      }

      i++;
    }
    ret = i;
    curl_easy_cleanup(curl);
  }
  else
  {
    mvprintw(0, 0, "%s", _("You have no feed yet."));
  }
  return ret;
}
