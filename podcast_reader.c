#include <string.h>
#include <unistd.h>
#include <stdbool.h>


#include <locale.h>
#include <libintl.h>

#include <ncurses.h>
#include <libxml/xmlreader.h>
#include <curl/curl.h>

#include "include/pr_const.h"
#include "include/gui.h"
#include "include/fileop.h"
#include "include/xmlop.h"
#include "include/strop.h"

#define _(STRING) gettext(STRING)

extern char READER_PATHS[4][80];

static void my_init_screen(void)
{
  setlocale(LC_ALL, "");
  bindtextdomain ("podcast_reader", READER_PATHS[LOCALE_PATH]);
  textdomain ("podcast_reader");

  initscr();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  curs_set(0);
}


int main(void)
{
  struct MyArray menu;
    menu.count = 0;
    menu.ptr = NULL;

  struct State state;
    state.level = FEED_LIST;
    state.current_feed = 0;
    state.highlight = 1;

  char feed_file[80] = {0};

  struct Download_data download_data;

  set_paths();
  my_init_screen();
  while ( 0 == (state.rss_count = get_feed_list()) )
  {
    add_url();
  }

  LIBXML_TEST_VERSION
  menu.count = state.rss_count;
  menu.ptr = malloc(menu.count * ITEMSIZE);

  bool level_change = true;
  int key_press = ERR;

  CURLM * multi_handle = curl_multi_init();
  int active_downloads;

  do
  {
    switch (state.level)
    {
      case FEED_LIST:

        if ( level_change )
        {
          menu.count = state.rss_count;
          menu.ptr = realloc(menu.ptr, menu.count * ITEMSIZE);
          memset(menu.ptr,'\0', menu.count * ITEMSIZE);
          for(unsigned int i = 0; i < state.rss_count; ++i)
          {
            sprintf(feed_file, READER_PATHS[FEED_TEMPLATE], i);
            copy_single_content(feed_file, 2, "title", 1, menu.ptr + ITEMSIZE * i, ITEMSIZE - 1);
          }
          state.highlight = state.current_feed + 1;
          level_change = false;
        }
          print_menu(menu.ptr, menu.count, state.highlight);
      break;

    case EPISODE_LIST:
        if ( level_change )
        {
          state.current_feed = state.highlight -1;
          strncpy(download_data.id3.artist , menu.ptr + ITEMSIZE * state.current_feed, 29);
          download_data.id3.artist[30] = '\0';
          state.highlight = 1;
          sprintf(feed_file, READER_PATHS[FEED_TEMPLATE], state.current_feed);
          menu.count = count_nodes(feed_file, "item", 2);
          menu.ptr = realloc(menu.ptr, menu.count * ITEMSIZE);
          memset(menu.ptr,'\0', menu.count * ITEMSIZE);
          read_feed(feed_file, menu.ptr);
          level_change = false;
        }
      print_menu(menu.ptr, menu.count, state.highlight);

      /*show_description(feed_file, state.highlight);*/

    break;

    case SELECTED_EPISODE:
      sprintf(feed_file, READER_PATHS[FEED_TEMPLATE], state.current_feed);
      strncpy(download_data.id3.album, menu.ptr + ITEMSIZE * (state.highlight - 1), 30);
      download_data.id3.album[31] = '\0';
      strncpy(download_data.id3.title, menu.ptr + ITEMSIZE * (state.highlight - 1), 30);
      download_data.id3.title[31] = '\0';
      download_data.url = get_enclosure(feed_file, state.highlight);

      clear();
        mvprintw(0,0, "%s: %s", _("Podcast"), download_data.id3.artist);
        mvprintw(1,0, "%s: %s", _("Episode"), download_data.id3.album);
        mvprintw(2,0, "%s: %s", _("URL"), download_data.url);
      refresh();

      set_download_destination(&download_data);
      char full_path[120];
      strcpy(full_path, download_data.dest_dir);
      strcat(full_path, "/");
      strcat(full_path, download_data.dest_file);
      FILE * dest_file = fopen(download_data.dest_file, "w");

      CURL *curl = curl_easy_init();
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, dest_file);
      curl_easy_setopt(curl, CURLOPT_URL, download_data.url);
      curl_multi_add_handle(multi_handle, curl);


      state.level = EPISODE_LIST;
    break;

    default:
    break;
  }

    while (ERR == (key_press = getch()))
    {
      curl_multi_perform(multi_handle, &active_downloads);
    }
    switch (key_press)
    {
      case 10:
        ++state.level;
        level_change = true;
        break;

      case KEY_UP:
        if (state.highlight > 1)
        {
          --state.highlight;
        }
        break;

      case KEY_DOWN:
        if(state.highlight < menu.count)
        {
          ++state.highlight;
        }
        break;

      case 'q':
        --state.level;
          state.highlight = 1;
          level_change = true;
          break;

        default:
          break;
    }




  }
  while(state.level > PROGRAM_EXIT);
  clear();
  mvprintw(LINES - 1, 0, "%s", _("Wait for downloads..."));
  refresh();

  while ( active_downloads )
  {
    curl_multi_perform(multi_handle, &active_downloads);
  }


  free(menu.ptr);
  curl_multi_cleanup(multi_handle);
  endwin();
  return 0;
}


