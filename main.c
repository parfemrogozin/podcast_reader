#include <string.h>
#include <locale.h>
#include <ncurses.h>
#include <libxml/xmlreader.h>
#include "fileop.h"


void print_menu(const char * titles, int lines, int highlight)
{
  int page = 5;
  for(int i = 0; i < page || i < lines; ++i)
  {
    if(highlight == i + 1)
    {
      attron(A_REVERSE);
      mvprintw(i, 0, "%s", titles + ITEMSIZE * i);
      attroff(A_REVERSE);
    }
    else
    {
    mvprintw(i, 0, "%s", titles  + ITEMSIZE * i);
    }
  }
  refresh();
}

int read_feed(xmlTextReaderPtr reader, char * menu_items)
{
  const xmlChar * item = (const xmlChar *)"item";
  const xmlChar * title = (const xmlChar *)"title";
  int ret;
  int depth;
  int type;
  const xmlChar * tag_name = NULL;
  const int min_depth = 2;
  int inside = 0;
  int is_title = 0;
  int i = 0;


  ret = xmlTextReaderRead(reader);
  if (ret != 1)
  {
    return -1;
  }
  while (ret == 1)
  {
    ret = xmlTextReaderRead(reader);
    depth = xmlTextReaderDepth(reader);
    if (depth < min_depth)
    {
      continue;
    }
    type = xmlTextReaderNodeType(reader);
    if (inside == 0 && type == 1)
    {
      tag_name = xmlTextReaderConstName(reader);
      if (!xmlStrcmp(tag_name, item))
      {
        inside = 1;
        continue;
      }
    }
    if (inside == 1 && type == 1)
    {
      tag_name = xmlTextReaderConstName(reader);
      if (!xmlStrcmp(tag_name, title))
      {
        is_title = 1;
        continue;
      }
    }
    if (is_title == 1 && type == 3)
    {
      strcpy (menu_items + ITEMSIZE * i, (char *) xmlTextReaderConstValue(reader));
      ++i;
      is_title = 0;
    }
  }
  return 0;
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

    case 'q':
      choice = -1;
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
  xmlFreeTextReader(reader);
  return item_count;
}


int main(void)
{
  int lines = 0;
  char * menu_items;
  xmlTextReaderPtr * readers;
  int files = 0;
  int choice = 0;
  int highlight = 1;
  const xmlChar * search_term = (const xmlChar *)"title";
  int ret = 0;


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
  for(int i = 0; i < files; ++i)
  {
    readers[i] = xmlReaderForFile(file_list + ITEMSIZE * i, NULL,0);
    strcpy (menu_items + ITEMSIZE * i, (char *) read_single_value(readers[i], search_term));
  }

  do
  {
    print_menu(menu_items, lines, highlight);
    choice = read_controls(&highlight, lines);
    print_menu(menu_items, lines, highlight);

    if(choice > 0)
    {
      int i = choice -1;
      lines = count_items(readers[i]);
      readers[i] = xmlReaderForFile(file_list + ITEMSIZE * i, NULL,0);
      menu_items = realloc(menu_items, lines * ITEMSIZE);
      ret = read_feed(readers[i], menu_items);
      if (ret != 0)
      {
        break;
      }
    }
  }
  while(choice > -1);

  endwin();
  for(int i = 0; i < files; ++i)
  {
    xmlFreeTextReader(readers[i]);
    remove(file_list + ITEMSIZE * i);
  }
  free(menu_items);
  free(readers);
  free(file_list);
}


