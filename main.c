#include <locale.h>
#include <ncurses.h>
#include <libxml/xmlreader.h>
#include "fileop.h"


void print_menu(const xmlChar ** titles, int lines, int highlight)
{
  int y = 0;
  for(int i = 0; i < lines; ++i)
  {
    if(highlight == i + 1)
    {
      attron(A_REVERSE);
      mvprintw(y, 0, "%s", titles[i]);
      attroff(A_REVERSE);
    }
    else
    {
    mvprintw(y, 0, "%s", titles[i]);
    }
    ++y;
  refresh();
  }
}

const xmlChar * read_single_value(xmlTextReaderPtr reader, const xmlChar * search_term)
{
  int ret;
  int type;
  int depth;
  const int min_depth = 2;
  const xmlChar * tag_name = NULL;
  const xmlChar * value = NULL;

  ret = xmlTextReaderRead(reader);
  while (ret == 1)
  {
    depth = xmlTextReaderDepth(reader);
    if (depth >= min_depth)
    {
      type = xmlTextReaderNodeType(reader);
      if (type == 1)
      {
        tag_name = xmlTextReaderConstName(reader);
      }
      else if (type == 15)
      {
        tag_name = NULL;
      }
      else if ((type == 3 || type == 4) && !xmlStrcmp(tag_name, search_term))
      {
        value = xmlTextReaderConstValue(reader);
        break;
      }
    }
    ret = xmlTextReaderRead(reader);
  }
  return value;
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

    default:
    break;
  }
return choice;

}

int count_items(xmlTextReaderPtr reader)
{
  int item_count = 0;
  const xmlChar * search_term = (const xmlChar *)"item";
  const xmlChar * tag_name = NULL;
  int depth;
  int type;
  const int min_depth = 2;
  int ret;

  do
  {
    ret = xmlTextReaderRead(reader);
    depth = xmlTextReaderDepth(reader);
    if (depth < min_depth)
    {
      continue;
    }
    type = xmlTextReaderNodeType(reader);
    if (type == 1)
    {
      tag_name = xmlTextReaderConstName(reader);
      if (!xmlStrcmp(tag_name, search_term))
      {
        ++item_count;
      }

    }
  }
  while (ret == 1);
  return item_count;
}


int main(void)
{
  int lines = 0;
  const xmlChar ** menu_items;
  xmlTextReaderPtr * readers;
  int files = 0;
  int choice = 0;
  int highlight = 1;
  const xmlChar * search_term = (const xmlChar *)"title";


  setlocale(LC_ALL, "");
  LIBXML_TEST_VERSION

  initscr();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  curs_set(0);

  mvprintw(23, 0, "%s", "Stahuji RSS");
  refresh();
  char * file_list = create_feed_list(&files);
  move(23,0);
  clrtoeol();
  refresh();
  lines = files;
  menu_items = malloc(lines * sizeof(xmlChar *));
  readers = malloc(files * sizeof(xmlTextReaderPtr));
  for(int i = 0; i < files; ++i)
  {
    readers[i] = xmlReaderForFile(file_list + ITEMSIZE * i, NULL,0);
    menu_items[i] = read_single_value(readers[i], search_term);
  }

  print_menu(menu_items, lines, highlight);
  while(1)
  {
    choice = read_controls(&highlight, lines);
    print_menu(menu_items, lines, highlight);

    if(choice != 0)
    {
      lines = count_items(readers[choice - 1]);
      break;
    }
  }

  endwin();
  --choice;
  printf("%s má %d položek.\n", file_list + ITEMSIZE * choice, lines);
  for(int i = 0; i < files; ++i)
  {
    xmlFreeTextReader(readers[i]);
    remove(file_list + ITEMSIZE * i);
  }
  free(menu_items);
  free(readers);
  free(file_list);
}


