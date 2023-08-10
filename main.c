#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>

#include <locale.h>
#include <libintl.h>

#include <ncurses.h>
#include <libxml/xmlreader.h>

#include "fileop.h"
#include "xmlop.h"
#include "strop.h"

#define _(STRING) gettext(STRING)

extern char READER_PATHS[4][80];

enum Level
{
  PROGRAM_EXIT = 0,
  FEED_LIST = 1,
  EPISODE_LIST = 2,
  SELECTED_EPISODE = 3
};

enum Commands
{
  QUIT = -1,
  ADD_FEED = -2,
  GET_INFO = -3,
  SEARCH = -4
};

struct MyArray
{
  unsigned int count;
  char * ptr;
};

struct State
{
  enum Level level;
  unsigned int highlight;
  int choice;
  unsigned int current_feed;
};

static void my_init_screen(void)
{
  setlocale(LC_ALL, "");
  bindtextdomain ("podcast_reader", READER_PATHS[LOCALE_PATH]);
  textdomain ("podcast_reader");

  initscr();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  curs_set(0);
}

void print_menu(struct MyArray menu, unsigned int highlight)
{
  unsigned int page_size = LINES - 1;
  unsigned int page_start = ((highlight - 1) / page_size) * page_size;
  erase();
  for(unsigned int i = 0; i < page_size && i < menu.count; ++i)
  {
    int array_step = ITEMSIZE * (i + page_start);
    if ((i + page_start) <  menu.count)
    {
      if(highlight == i + page_start + 1)
      {
        attron(A_REVERSE);
        mvprintw(i, 0, "%s", menu.ptr + array_step);
        attroff(A_REVERSE);
      }
      else
      {
        mvprintw(i, 0, "%s", menu.ptr  + array_step);
      }
    }
  }
  refresh();
}



int read_controls(unsigned int * highlight, unsigned int lines)
{
  int c = getch();
  int choice = 0;

  switch(c)
  {
    case KEY_UP:
      if (*highlight > 1)
      {
        --*highlight;
      }
      break;

    case KEY_DOWN:
      if(*highlight < lines)
      {
        ++*highlight;
      }
    break;

    case 10:
      choice = *highlight;
    break;

    case 'q':
      choice = QUIT;
    break;

    case 'a':
      choice = ADD_FEED;
    break;

    case 'i':
      choice = GET_INFO;
    break;

    case '/':
      choice = SEARCH;
    break;

    default:
    break;
  }
return choice;
}

void show_description(char * rss_file, const int highlight)
{
  char description[SCREENSIZE + 1];
  copy_single_content(rss_file, 3, "description", highlight, description, SCREENSIZE - 1);
  strip_html(description);
  replace_multi_space_with_single_space(description);
  clear();
  mvprintw(0, 0, "%s", description);
  refresh();
}


int main(void)
{
  struct MyArray menu;
    menu.count = 0;
    menu.ptr = NULL;

  struct MyArray rss;
    rss.count = 0;
    rss.ptr = NULL;


  struct State state;
    state.level = FEED_LIST;
    state.highlight = 1;
    state.choice = -1;
    state.current_feed = 0;
    rss.count = 0;

  struct Download_data download_data;
  pthread_t downloader_thread_id;
  struct mq_attr dq_attr;
  mqd_t download_queue;
  dq_attr.mq_flags = 0;
  dq_attr.mq_maxmsg = 10;
  dq_attr.mq_msgsize = sizeof(download_data);
  dq_attr.mq_curmsgs = 0;

  set_paths();
  my_init_screen();

  LIBXML_TEST_VERSION

  download_queue = mq_open (QUEUENAME,  O_WRONLY | O_CREAT,  0600, &dq_attr);
  pthread_create(&downloader_thread_id, NULL, start_downloader, NULL);

  mvprintw(LINES-1, 0, "%s", _("Downloading RSS"));
  refresh();
  rss.ptr = create_feed_list(&rss.count);
  move(LINES-1,0);
  clrtoeol();
  refresh();
  menu.count = rss.count;
  menu.ptr = malloc(menu.count * ITEMSIZE);

  do
  {
    switch (state.level)
    {

      case FEED_LIST:
        menu.count = rss.count;
        if (state.choice < 0)
        {
          menu.ptr = realloc(menu.ptr, menu.count * ITEMSIZE);
          memset(menu.ptr,'\0', menu.count * ITEMSIZE);
          for(unsigned int i = 0; i < rss.count; ++i)
          {
            copy_single_content(rss.ptr + ITEMSIZE * i, 2, "title", 1, menu.ptr + ITEMSIZE * i, ITEMSIZE - 1);
          }
          state.highlight = state.current_feed + 1;
        }
        print_menu(menu, state.highlight);
      break;

    case EPISODE_LIST:
      if (state.choice > 0)
      {
        state.current_feed = state.choice -1;
        strncpy(download_data.id3.artist , menu.ptr + ITEMSIZE * state.current_feed, 30);

        menu.count = count_nodes(rss.ptr + ITEMSIZE * state.current_feed, "item", 2);
        menu.ptr = realloc(menu.ptr, menu.count * ITEMSIZE);
        memset(menu.ptr,'\0', menu.count * ITEMSIZE);

        read_feed(rss.ptr + ITEMSIZE * state.current_feed, menu.ptr);
      }
      print_menu(menu, state.highlight);
      if (state.choice == -3)
      {
        show_description(rss.ptr + ITEMSIZE * state.current_feed, state.highlight);
      }
    break;

    case SELECTED_EPISODE:
      strncpy(download_data.id3.album, menu.ptr + ITEMSIZE * (state.highlight - 1), 30);
      strncpy(download_data.id3.title, menu.ptr + ITEMSIZE * (state.highlight - 1), 8);
      download_data.url = get_enclosure(rss.ptr + ITEMSIZE * state.current_feed, state.choice);

      clear();
        mvprintw(0,0, "%s: %s", _("Podcast"), download_data.id3.artist);
        mvprintw(1,0, "%s: %s", _("Episode"), download_data.id3.album);
        mvprintw(2,0, "%s: %s", _("URL"), download_data.url);
      refresh();

      mq_send(download_queue, (char *) &download_data, sizeof(download_data), 1);

      state.level = EPISODE_LIST;
    break;

    default:
    break;
  }

    state.choice = read_controls(&state.highlight, menu.count);


    if(state.choice > 0)
    {
      ++state.level;
      if (state.level < 3) state.highlight = 1;
    }
    if (state.choice == -1)
    {
      --state.level;
      state.highlight = 1;
    }
    if (state.choice == -2)
    {
      add_url();
      free(rss.ptr);
      mvprintw(LINES-1, 0, "%s", _("Downloading RSS"));
      rss.ptr = create_feed_list(&rss.count);
    }
    if (state.choice == -4)
    {
      echo();
      mvprintw(LINES-1, 0,"%s", _("Find: "));
      char search_term[80];
      getstr(search_term);
      noecho();
      state.highlight = find_string_in_array(menu.ptr, search_term, 0, menu.count);
    }

  }
  while(state.level > PROGRAM_EXIT);


  clear();
  mvprintw(LINES-2,0, "%s", _("Wait for downloads..."));
  refresh();
  free(menu.ptr);
  for (unsigned int i = 0; i < rss.count; i++)
  {
    unlink(rss.ptr + ITEMSIZE * i);
  }
  free(rss.ptr);

  mq_send(download_queue, (char *) &download_data, sizeof(download_data), 0);
  pthread_join(downloader_thread_id, NULL);
  mq_close(download_queue);
  mq_unlink(QUEUENAME);
  endwin();

  return 0;
}


