#include <locale.h>
#include <ncurses.h>
#include <libxml/xmlreader.h>
#include "fileop.h"


void print_menu(char ** titles, int lines, int highlight)
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
    if (depth < min_depth)
    {
    ret = xmlTextReaderRead(reader);
    continue;
    }
    type = xmlTextReaderNodeType(reader);
    if (type == 1)
    {
      tag_name = xmlTextReaderConstName(reader);
    }
    if (type == 15)
    {
      tag_name = NULL;
    }
    if ((type == 3 || type == 4) && !xmlStrcmp(tag_name, search_term))
    {
      value = xmlTextReaderConstValue(reader);
      break;
    }
    ret = xmlTextReaderRead(reader);
  }
  return value;
}

char ** read_list(char ** file_list, int lines)
{
  const xmlChar * search_term = (const xmlChar *)"title";
  char ** titles = malloc(lines * sizeof(xmlChar*));
  for(int i = 0; i < lines; ++i)
  {
    xmlTextReaderPtr reader = xmlReaderForFile(file_list[i], NULL,0);
    titles[i] = (char *)read_single_value(reader, search_term);
  }
  return titles;
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
  int choice = 0;
  int highlight = 1;
  int files = 0;
  xmlTextReaderPtr reader;


  setlocale(LC_ALL, "");
  LIBXML_TEST_VERSION

  initscr();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  curs_set(0);

  mvprintw(23, 0, "%s", "Stahuji RSS");
  refresh();
  char ** file_list = create_feed_list(&lines);
  move(23,0);
  clrtoeol();
  mvprintw(23, 0, "%s", "Načítám položky");
  refresh();
  char ** menu_items = read_list(file_list, lines);
  files = lines;
  move(23,0);
  clrtoeol();

  print_menu(menu_items, lines, highlight);
  while(1)
  {
    choice = read_controls(&highlight, lines);
    print_menu(menu_items, lines, highlight);

    if(choice != 0)
    {
    reader = xmlReaderForFile(file_list[choice-1], NULL,0);
    lines = count_items(reader);
    break;
    }
  }

  endwin();
  printf("%s má %d položek.\n", file_list[choice-1], lines);
  free(menu_items);
  xmlFreeTextReader(reader);
  for (int i = 0; i < files; ++i)
  {
    remove(file_list[i]);
    free(file_list[i]);
  }
  free(file_list);


}


