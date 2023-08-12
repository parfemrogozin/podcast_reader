#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED
void print_menu(char * menu_ptr, unsigned int menu_count, unsigned int highlight);
int read_controls(unsigned int * highlight, unsigned int lines);
void show_description(char * rss_file, const int highlight);
#endif /* GUI_H_INCLUDED */