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

void print_menu(const char * titles, int lines, int highlight)
{
  int page_size = LINES - 1;
  int page_start = ((highlight - 1) / page_size) * page_size;
  erase();
  for(int i = 0; i < page_size && i < lines; ++i)
  {
    int array_step = ITEMSIZE * (i + page_start);
    if ((i + page_start) < lines)
    {
      if(highlight == i + page_start + 1)
      {
        attron(A_REVERSE);
        mvprintw(i, 0, "%s", titles + array_step);
        attroff(A_REVERSE);
      }
      else
      {
        mvprintw(i, 0, "%s", titles  + array_step);
      }
    }
  }
  refresh();
}



int read_controls(int * highlight, int lines)
{
  int c;
  int choice = 0;
  c = getch();

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
      choice = -1;
    break;

    case 'a':
      choice = -2;
    break;

    case 'i':
      choice = -3;
    break;

    case '/':
      choice = -4;
    break;

    default:
    break;
  }
return choice;

}

int main(void)
{
  int lines = 0;
  char * menu_items;
  char * item;
  char search_term[80];
  int files = 0;
  int choice = -1;
  int highlight = 1;
  int level = 1;
  int current_feed = 0;
  struct Download_data download_data;

  pthread_t downloader_thread_id;
  struct mq_attr dq_attr;

  mqd_t download_queue;
  dq_attr.mq_flags = 0;
  dq_attr.mq_maxmsg = 10;
  dq_attr.mq_msgsize = sizeof(download_data);
  dq_attr.mq_curmsgs = 0;

  set_paths();

  setlocale(LC_ALL, "");
  bindtextdomain ("podcast_reader", READER_PATHS[LOCALE_PATH]);
  textdomain ("podcast_reader");

  LIBXML_TEST_VERSION

  download_queue = mq_open (QUEUENAME,  O_WRONLY | O_CREAT,  0600, &dq_attr);
  pthread_create(&downloader_thread_id, NULL, start_downloader, NULL);


  initscr();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  curs_set(0);

  mvprintw(LINES-1, 0, "%s", _("Downloading RSS"));
  refresh();
  char * file_list = create_feed_list(&files);
  move(LINES-1,0);
  clrtoeol();
  refresh();
  lines = files;
  menu_items = malloc(lines * ITEMSIZE);

  do
  {
    switch (level)
    {

      case 1:
        lines = files;
        if (choice < 0)
        {
          menu_items = realloc(menu_items, lines * ITEMSIZE);
          memset(menu_items,'\0', lines * ITEMSIZE);
          for(int i = 0; i < files; ++i)
          {
            item = read_single_value(file_list + ITEMSIZE * i, (const xmlChar *)"title");
            strncpy(menu_items + ITEMSIZE * i, item, ITEMSIZE - 2);
            free(item);
          }
          highlight = current_feed + 1;
        }
        print_menu(menu_items, lines, highlight);
      break;

    case 2:
      if (choice > 0)
      {
        current_feed = choice -1;
        strncpy(download_data.id3.artist , menu_items + ITEMSIZE * current_feed, 30);

        lines = count_items(file_list + ITEMSIZE * current_feed);
        menu_items = realloc(menu_items, lines * ITEMSIZE);
        memset(menu_items,'\0', lines * ITEMSIZE);

        read_feed(file_list + ITEMSIZE * current_feed, menu_items);
      }
      print_menu(menu_items, lines, highlight);
      if (choice == -3)
      {

        char * description = (char *) get_description(file_list + ITEMSIZE * current_feed, highlight);
        strip_html(description);
        replace_multi_space_with_single_space(description);
        clear();
        mvprintw(0, 0, "%s", description);
        refresh();
        free(description);
      }
    break;

    case 3:
      strncpy(download_data.id3.album, menu_items + ITEMSIZE * (highlight - 1), 30);
      strncpy(download_data.id3.title, menu_items + ITEMSIZE * (highlight - 1), 8);
      download_data.url = (char *) get_enclosure(file_list + ITEMSIZE * current_feed, choice);

      clear();
        mvprintw(0,0, "%s: %s", _("Podcast"), download_data.id3.artist);
        mvprintw(1,0, "%s: %s", _("Episode"), download_data.id3.album);
        mvprintw(2,0, "%s: %s", _("URL"), download_data.url);
      refresh();

      mq_send(download_queue, (char *) &download_data, sizeof(download_data), 1);

      level = 2;
    break;

    default:
    break;
  }

    choice = read_controls(&highlight, lines);


    if(choice > 0)
    {
      ++level;
      if (level < 3) highlight = 1;
    }
    if (choice == -1)
    {
      --level;
      highlight = 1;
    }

    if (choice == -2)
    {
      add_url();
      free(file_list);
      mvprintw(LINES-1, 0, "%s", _("Downloading RSS"));
      file_list = create_feed_list(&files);
    }

    if (choice == -4)
    {
      echo();
      mvprintw(LINES-1, 0,"%s", _("Find: "));
      getstr(search_term);
      noecho();
      highlight = find_string_in_array(menu_items, search_term, 0, lines);
    }

  }
  while(level > 0);


  clear();
  mvprintw(LINES-2,0, "%s", _("Wait for downloads..."));
  refresh();
  free(menu_items);
  for (int i = 0; i < files; i++)
  {
    unlink(file_list + ITEMSIZE * i);
  }
  free(file_list);

  mq_send(download_queue, (char *) &download_data, sizeof(download_data), 0);
  pthread_join(downloader_thread_id, NULL);
  mq_close(download_queue);
  mq_unlink(QUEUENAME);
  endwin();

  return 0;
}


