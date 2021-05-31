int count_items(char * rss_file);
int read_feed(char * rss_file, char * menu_items);
char * get_enclosure(char * rss_file, int position);
char * get_description(char * rss_file, int position);
char * read_single_value(char * rss_file, const xmlChar * search_term);
