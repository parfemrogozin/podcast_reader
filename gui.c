#include <ncurses.h>

#include "include/pr_const.h"
#include "include/xmlop.h"
#include "include/strop.h"
#include "include/gui.h"


void print_menu(char * menu_ptr, unsigned int menu_count, unsigned int highlight)
{
  unsigned int page_size = LINES - 1;
  unsigned int page_start = ((highlight - 1) / page_size) * page_size;
  erase();
  for(unsigned int i = 0; i < page_size && i < menu_count; ++i)
  {
    int array_step = ITEMSIZE * (i + page_start);
    if ((i + page_start) <  menu_count)
    {
      if(highlight == i + page_start + 1)
      {
        attron(A_REVERSE);
        mvprintw(i, 0, "%s", menu_ptr + array_step);
        attroff(A_REVERSE);
      }
      else
      {
        mvprintw(i, 0, "%s", menu_ptr  + array_step);
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
      choice = BACK;
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
