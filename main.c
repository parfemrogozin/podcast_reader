#include <string.h>
#include <locale.h>
#include <ncurses.h>
#include <libxml/xmlreader.h>
#include <pthread.h>

#include "fileop.h"
#include "xmlop.h"
#include "strop.h"

#define REWIND_READER() \
xmlFreeTextReader(readers[current_reader]);\
readers[current_reader] = xmlReaderForFile(file_list + ITEMSIZE * current_reader, NULL,0);\


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
  xmlTextReaderPtr * readers;
  int files = 0;
  int choice = -1;
  int highlight = 1;
  const xmlChar * search_term = (const xmlChar *)"title";
  int level = 1;
  int current_reader = 0;
  struct Download_data download_data;
  int cleared = 0;
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
  menu_items = malloc(lines * ITEMSIZE);
  readers = malloc(files * sizeof(xmlTextReaderPtr));

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
            readers[i] = xmlReaderForFile(file_list + ITEMSIZE * i, NULL,0);
            strncpy(menu_items + ITEMSIZE * i, (char *) read_single_value(readers[i], search_term), ITEMSIZE - 2);
          }
          highlight = current_reader + 1;
        }
        print_menu(menu_items, lines, highlight);
      break;

    case 2:
      if (choice > 0)
      {
        current_reader = choice -1;
        strncpy(download_data.directory, menu_items + ITEMSIZE * current_reader, ITEMSIZE - 1);
        remove_symbols(download_data.directory);
        replace_char(download_data.directory, ' ', '_');
        lines = count_items(readers[current_reader]);
        readers[current_reader] = xmlReaderForFile(file_list + ITEMSIZE * current_reader, NULL,0);
        menu_items = realloc(menu_items, lines * ITEMSIZE);
        memset(menu_items,'\0', lines * ITEMSIZE);
        read_feed(readers[current_reader], menu_items);
        readers[current_reader] = xmlReaderForFile(file_list + ITEMSIZE * current_reader, NULL,0);
      }
      print_menu(menu_items, lines, highlight);
      if (choice == -3)
      {
        REWIND_READER()
        char * description = (char *) get_description(readers[current_reader], highlight);
        strip_html(description);
        replace_multi_space_with_single_space(description);
        clear();
        mvprintw(0, 0, "%s", description);
        REWIND_READER()
        refresh();
      }
    break;

    case 3:
      if (thread_index < MAX_THREADS)
      {
        strncpy(download_data.filename, menu_items + ITEMSIZE * (highlight - 1), BASENAMESIZE);
        download_data.filename[BASENAMESIZE -1] = '\0';
        remove_symbols(download_data.filename);
        replace_char(download_data.filename, ' ', '_');
        strcat(download_data.filename, ".mp3");
        download_data.url = (char *) get_enclosure(readers[current_reader], choice);
        struct Download_data * ddataptr = & download_data;
        pthread_create(&download_thread[thread_index], NULL, threaded_download, ddataptr);
        thread_index++;
        readers[current_reader] = xmlReaderForFile(file_list + ITEMSIZE * current_reader, NULL,0);
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
      if (cleared == 0 && level < 2)
      {
        for(int i = 0; i < files; ++i)
        {
          xmlFreeTextReader(readers[i]);
        }
        cleared = 1;
      }
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
  free(readers);
  free(file_list);

  puts("Čekám, než se dokončí stahování...");
  for(size_t i = 0; i< thread_index; i++)
  {
    pthread_join(download_thread[i], NULL);
  }

  return 0;
}


