#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <ncurses.h>
#include <libxml/xmlreader.h>
#include <pthread.h>

#include "fileop.h"
#include "xmlop.h"
#include "strop.h"

const size_t MAX_THREADS = 16;

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
  int files = 0;
  int choice = -1;
  int highlight = 1;
  int level = 1;
  int current_feed = 0;

  struct Download_data download_data;
  pthread_t download_thread[MAX_THREADS];
  size_t thread_index = 0;

  setlocale(LC_ALL, "");
  LIBXML_TEST_VERSION

  initscr();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  curs_set(0);

  mvprintw(LINES-1, 0, "%s", "Stahuji RSS");
  refresh();
  char * file_list = create_feed_list(&files);
  move(LINES-1,0);
  clrtoeol();
  refresh();
  lines = files;
  menu_items = (char *) malloc(lines * ITEMSIZE);

  do
  {
    switch (level)
    {

      case 1:
        lines = files;
        if (choice < 0)
        {
          menu_items = (char *) realloc(menu_items, lines * ITEMSIZE);
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
        strncpy(download_data.directory, menu_items + ITEMSIZE * current_feed, ITEMSIZE - 1);

        lines = count_items(file_list + ITEMSIZE * current_feed);
        menu_items = (char *) realloc(menu_items, lines * ITEMSIZE);
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
        free(description);
        refresh();
      }
    break;

    case 3:
      if (thread_index < MAX_THREADS)
      {
        remove_symbols(download_data.directory);
        replace_char(download_data.directory, ' ', '_');

        strncpy(download_data.filename, menu_items + ITEMSIZE * (highlight - 1), BASENAMESIZE);
        download_data.filename[BASENAMESIZE -1] = '\0';
        remove_symbols(download_data.filename);
        replace_char(download_data.filename, ' ', '_');
        strcat(download_data.filename, ".mp3");

        download_data.url = (char *) get_enclosure(file_list + ITEMSIZE * current_feed, choice);

        clear();
        mvprintw(0, 0, "%s", download_data.directory);
        mvprintw(1, 0, "%s", download_data.filename);
        mvprintw(2, 0, "%s", download_data.url);
        refresh();


        /*
        struct Download_data * ddataptr = & download_data;
        pthread_create(&download_thread[thread_index], NULL, threaded_download, ddataptr);
        thread_index++;
        */
      }
      else
      {
        mvprintw(LINES-1, 0, "%s", "Čekám, než se dokončí stahování...");
        refresh();
        for(size_t i = 0; i< thread_index; i++)
        {
          pthread_join(download_thread[i], NULL);
        }
        move(LINES-1,0);
        clrtoeol();
        refresh();
      }
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
      mvprintw(LINES-1, 0, "%s", "Stahuji RSS");
      file_list = create_feed_list(&files);
    }
  }
  while(level > 0);

  endwin();
  free(menu_items);
  for (int i = 0; i < files; i++)
  {
    unlink(file_list + ITEMSIZE * i);
  }
  free(file_list);

  for(size_t i = 0; i< thread_index; i++)
  {
    pthread_join(download_thread[i], NULL);
  }
  return 0;
}


