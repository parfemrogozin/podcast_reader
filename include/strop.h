#ifndef STROP_H_INCLUDED
#define STROP_H_INCLUDED
void remove_symbols(char *str);
void replace_char(char* str, char find, char replace);
void strip_html(char* str);
void replace_multi_space_with_single_space(char *str);
void sanitize_filename(char *str);
int find_string_in_array(char * str_array, char * string, int start, int lines);
#endif /* STROP_H_INCLUDED */