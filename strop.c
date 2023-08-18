#define _GNU_SOURCE
#include <string.h>
#include "include/strop.h"

void remove_symbols(char *str)
{
  char *dest = str;
  int char_len;

  while ( *str != '\0' )
  {
    if ( *str > 0)
    {
      if ( *str < 47 && *str != 32) /* 32 = SPACE */
      {
        str++;
      }
      else if ( *str > 57 &&  *str < 65) /* 65 = LATIN CAPITAL LETTER A */
      {
        str++;
      }
      else if ( *str > 90  && *str < 97) /* 97 = LATIN SMALL LETTER A */
      {
        str++;
      }
      else if ( *str > 122) /* 122 = LATIN SMALL LETTER Z, 127 = <DEL> */
      {
        str++;
      }
      else /* LETTERS AND NUMBERS*/
      {
        *dest++ = *str++;
      }
    }
    else
    {
      *dest++ = *str++;
    }

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


int find_string_in_array(char * str_array, char * string, int start, int lines)
{
  int position = 0;
  for(int i = start; i < lines; ++i)
  {
    if( strcasestr(str_array + 81 * i, string) )
    {
      position = i + 1;
      break;
    }
  }
  return position;
}
