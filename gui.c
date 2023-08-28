#include <ncurses.h>
#include <string.h>


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


void show_description(char * rss_file, const int highlight)
{
  char description[SCREENSIZE + 1] = {0};
  copy_single_content(rss_file, 3, "description", highlight, description, SCREENSIZE - 1);
  strip_html(description);
  replace_multi_space_with_single_space(description);
  clear();
  mvprintw(0, 0, "%s", description);
  refresh();
}
