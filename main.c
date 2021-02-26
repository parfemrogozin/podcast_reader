#include <string.h>
#include <locale.h>
#include <ncurses.h>
#include <libxml/xmlreader.h>
#include "fileop.h"

const xmlChar * get_enclosure(xmlTextReaderPtr reader, int position)
{
  const xmlChar * url = NULL;
  const xmlChar * search_tag = (const xmlChar *)"enclosure";
  const xmlChar * search_attribute = (const xmlChar *)"url";
  const int target_depth = 3;
  int ret, depth, type;
  int count = 0;
  const xmlChar * tag_name;

  ret = xmlTextReaderRead(reader);

  while (ret == 1)
  {
    ret = xmlTextReaderRead(reader);
    depth = xmlTextReaderDepth(reader);
    if (depth != target_depth) continue;
    type = xmlTextReaderNodeType(reader);
    if (type != 1) continue;
    tag_name = xmlTextReaderConstName(reader);
    if (!xmlStrcmp(tag_name, search_tag))
    {
      ++count;
      if (count == position)
      {
        url =  xmlTextReaderGetAttribute(reader, search_attribute);
      }
    }
  }
  return url;
}


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
    /* mvprintw(LINES-1, 0, "Start: %d, současná pozice: %d", page_start, highlight);*/
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
    xmlFreeTextReader(reader);
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
      strncpy (menu_items + ITEMSIZE * i, (char *) xmlTextReaderConstValue(reader), ITEMSIZE - 1);
      ++i;
      is_title = 0;
    }
  }
  xmlFreeTextReader(reader);
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
  int level = 1;
  int current_reader;
  int cleared = 0;


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
        menu_items = realloc(menu_items, lines * ITEMSIZE);
        for(int i = 0; i < files; ++i)
        {
          readers[i] = xmlReaderForFile(file_list + ITEMSIZE * i, NULL,0);
          strncpy (menu_items + ITEMSIZE * i, (char *) read_single_value(readers[i], search_term), ITEMSIZE - 2);
        }
        print_menu(menu_items, lines, highlight);
      break;

    case 2:
      if (choice > 0) current_reader = choice -1;
      lines = count_items(readers[current_reader]);
      readers[current_reader] = xmlReaderForFile(file_list + ITEMSIZE * current_reader, NULL,0);
      menu_items = realloc(menu_items, lines * ITEMSIZE);
      memset(menu_items,'\0', lines * ITEMSIZE);
      read_feed(readers[current_reader], menu_items);
      readers[current_reader] = xmlReaderForFile(file_list + ITEMSIZE * current_reader, NULL,0);
      print_menu(menu_items, lines, highlight);
    break;

    case 3:
      erase();
      mvprintw(LINES-1, 0, "%s", get_enclosure(readers[current_reader], choice));
      readers[current_reader] = xmlReaderForFile(file_list + ITEMSIZE * current_reader, NULL,0);
      refresh();
    break;

    default:
    break;
  }

    choice = read_controls(&highlight, lines);


    if(choice > 0)
    {
      ++level;
      highlight = 1;
    }
    if (choice < 0)
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

  }
  while(level > 0);

  endwin();
  /*for(int i = 0; i < files; ++i)
  {
    remove(file_list + ITEMSIZE * i);
  }*/
  free(menu_items);
  free(readers);
  free(file_list);
}


