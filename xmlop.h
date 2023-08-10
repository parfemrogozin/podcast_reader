unsigned int count_nodes(const char * file_name, const char * tag_name, const int level);
void copy_single_content(char * rss_file, const int min_depth, const char * enclosing_tag, unsigned int position, char * free_slot, int free_size);
char * get_enclosure(char * rss_file, const unsigned int position);
void read_feed(char * rss_file, char * menu_items);