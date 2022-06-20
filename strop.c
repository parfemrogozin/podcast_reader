#define _GNU_SOURCE
#include <string.h>
#include "strop.h"

void remove_symbols(char *str)
{
  char *dest = str;
  /* xmlGetUTF8Char () */
  while ( *str != '\0' )
  {
    if ( (*str < 48 && *str > 0 && *str != 32 ) || (*str < 65 && *str > 57 )  || (*str < 97 && *str > 90 ) || (*str > 122) )
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

void sanitize_filename(char *str)
{
  remove_symbols(str);
  replace_multi_space_with_single_space(str);
  replace_char(str, ' ', '_');
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
