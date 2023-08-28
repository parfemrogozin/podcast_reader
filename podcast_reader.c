#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <locale.h>
#include <libintl.h>
#include <sys/stat.h>



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
  nodelay(stdscr, FALSE);
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  curs_set(0);
}

static FILE * set_destination(struct CurlPrivate *private_data)
{
  strcpy(private_data->path, READER_PATHS[MUSIC_DIRECTORY]);

  char feed[30+1] = {0};
  strcpy(feed, private_data->feed);
  replace_char(feed, ' ', '_');
  strcat(private_data->path, feed);
  strcat(private_data->path, "/");
  mkdir(private_data->path, 0750);

  char episode[30+1+3+1] = {0};
  strcpy(episode, private_data->episode);
  replace_char(episode, ' ', '_');
  strcat(private_data->path, episode);
  strcat(private_data->path, ".mp3");
  FILE *dest_file = fopen(private_data->path, "w");
  return dest_file;
}

static void postprocess(CURLM * multi_handle)
{
  struct CURLMsg *handle_message;
  do
  {
    int waiting_messages = 0;
    handle_message = curl_multi_info_read(multi_handle, &waiting_messages);
    if( handle_message != NULL && (handle_message->msg == CURLMSG_DONE) )
    {
      CURL *easy_handle = handle_message->easy_handle;
      curl_multi_remove_handle(multi_handle, easy_handle);

      struct CurlPrivate *private;
      curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &private);
      mvprintw(LINES - 1, 0, "%s: %s", _("Finished:"), private->episode);
      refresh();
      char filename[30+1+3+1];
      char * end_of_path = strrchr(private->path, '/');
      strcpy(filename, end_of_path + 1);
      *end_of_path = '\0';
      chdir(private->path);
      remove_id3tags(filename);
      add_id3tags(filename, private->feed, private->episode);
      char split_command[120];
      sprintf(split_command, "mp3splt -Q -t 10.00 -o @f/@n2 -g r%%[@o,@N=1,@t=#t@N] %s", filename);
      system(split_command);
      unlink(filename);


      free(private);
      curl_easy_cleanup(easy_handle);
    }
  } while( handle_message != NULL );

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

  set_paths();
  my_init_screen();
  while ( 0 == (state.rss_count = get_feed_list()) )
  {
    add_url();
  }
  state.current_feed = 1;

  LIBXML_TEST_VERSION
  menu.count = state.rss_count;
  menu.ptr = malloc(menu.count * ITEMSIZE);

  bool level_change = true;
  int key_press = ERR;
  enum Command command = NO_COMMAND;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURLM * multi_handle = curl_multi_init();
  int active_downloads = 0;

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
          for(unsigned int slot = 1; slot <= state.rss_count; ++slot)
          {
            sprintf(feed_file, READER_PATHS[FEED_TEMPLATE], slot);
            copy_single_content(feed_file, 2, "title", 1, menu.ptr + ITEMSIZE * (slot - 1), ITEMSIZE - 1);
          }
          state.highlight = state.current_feed;
          level_change = false;
        }
        if ( command == SHOW_INFO )
        {
          command = NO_COMMAND;
        }
        if ( command == DELETE_FEED )
        {
          state.rss_count = del_url(state.highlight);
          char old_file[80];
          char new_file[80];
          sprintf(old_file, READER_PATHS[FEED_TEMPLATE], state.highlight);
          unlink(old_file);
          for (unsigned int feedno = state.highlight; feedno <= state.rss_count; feedno++)
          {
            sprintf(old_file, READER_PATHS[FEED_TEMPLATE], feedno + 1);
            sprintf(new_file, READER_PATHS[FEED_TEMPLATE], feedno);
            rename(old_file, new_file);
          }
          command = NO_COMMAND;
        }
        else
        {
          print_menu(menu.ptr, menu.count, state.highlight);
        }
      break;

    case EPISODE_LIST:
        if ( level_change )
        {
          state.current_feed = state.highlight;
          state.highlight = 1;
          sprintf(feed_file, READER_PATHS[FEED_TEMPLATE], state.current_feed);
          menu.count = count_nodes(feed_file, "item", 2);
          menu.ptr = realloc(menu.ptr, menu.count * ITEMSIZE);
          memset(menu.ptr,'\0', menu.count * ITEMSIZE);
          read_feed(feed_file, menu.ptr);
          level_change = false;
        }
        if ( command == SHOW_INFO )
        {
          show_description(feed_file, state.highlight);
          command = NO_COMMAND;
        }
        else
        {
          print_menu(menu.ptr, menu.count, state.highlight);
        }
    break;

    case SELECTED_EPISODE:
      sprintf(feed_file, READER_PATHS[FEED_TEMPLATE], state.current_feed);
      char feed_name_buffer[161] = {0};
      char episode_name_buffer[161] = {0};
      char *url = get_enclosure(feed_file, state.highlight); /* TO BE FREED*/
      copy_single_content(feed_file, 2, "title", 1, feed_name_buffer, 160);
      copy_single_content(feed_file, 3, "title", 1+state.highlight, episode_name_buffer, 160);
      sanitize(feed_name_buffer);
      sanitize(episode_name_buffer);

      clear();
        mvprintw(0,0, "%s: %s", _("Podcast"),  feed_name_buffer);
        mvprintw(2,0, "%s: %s", _("Episode"), episode_name_buffer);
        mvprintw(4,0, "%s: %s", _("URL"), url);
      refresh();

      struct CurlPrivate *private_data = calloc(1, sizeof(struct CurlPrivate));
      strncpy(private_data->feed, feed_name_buffer, 30);
      strncpy(private_data->episode, episode_name_buffer, 30);



      FILE * dest_file = set_destination(private_data);


      nodelay(stdscr, TRUE);


      CURL *curl = curl_easy_init();
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, dest_file);
      curl_easy_setopt(curl, CURLOPT_URL, url);
      curl_easy_setopt(curl, CURLOPT_PRIVATE, private_data);
      curl_multi_add_handle(multi_handle, curl);
      free(url);

      level_change = false;
      state.level = EPISODE_LIST;
    break;

    default:
    break;
    }

    while (ERR == (key_press = getch()))
    {
      CURLMcode return_code_curl;
      return_code_curl = curl_multi_perform(multi_handle, &active_downloads);
      if ( return_code_curl == CURLM_OK )
      {
        curl_multi_wait(multi_handle, NULL, 0, 100, NULL);
        postprocess(multi_handle);
        if (active_downloads == 0)
        {
          nodelay(stdscr, FALSE);
        }
      }
      else
      {
        state.level = PROGRAM_EXIT;
        fprintf(stderr, "curl_multi failed, code %d.\n", return_code_curl);
        break;
      }
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

      case 'i':
        command = SHOW_INFO;
        break;

      case 'd':
        command = DELETE_FEED;
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
    int numfds;
    curl_multi_perform(multi_handle, &active_downloads);
    curl_multi_wait(multi_handle, NULL, 0, 375, &numfds);
    postprocess(multi_handle);
  }
  curl_multi_cleanup(multi_handle);
  curl_global_cleanup();
  free(menu.ptr);
  endwin();

  return 0;
}


