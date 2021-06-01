#include <string.h>
#include "strop.h"


void remove_symbols(char *str)
{
  char *dest = str;

  while (*str != '\0')
  {
    while (*str == '"' ||
           *str == ':' ||
           *str == '#' ||
           *str == '&')
    {
      str++;
    }
  *dest++ = *str++;
  }

  *dest = '\0';
}

void replace_char(char* str, char find, char replace)
{
  char *current_pos = strchr(str,find);
  while (current_pos)
  {
    *current_pos = replace;
    current_pos = strchr(current_pos,find);
  }
}

void strip_html(char* str)
{
  int inside = 0;
  while (*str)
  {
    if (*str == '<')
    {
      inside = 1;
    }
    if (*str == '>')
    {
      *str = ' ';
      inside = 0;
    }
    if (inside)
    {
      *str = ' ';
    }
    str++;
  }
}

void replace_multi_space_with_single_space(char *str)
{
  /* from https://stackoverflow.com/a/16790505 */
  char *dest = str;

  while (*str != '\0')
  {
    while (*str == ' ' && *(str + 1) == ' ')
    {
      str++;
    }
  *dest++ = *str++;
  }

  *dest = '\0';
}
