const xmlChar * get_enclosure(xmlTextReaderPtr reader, int position);
const xmlChar * get_description(xmlTextReaderPtr reader, int position);
int read_feed(xmlTextReaderPtr reader, char * menu_items);
const xmlChar * read_single_value(xmlTextReaderPtr reader, const xmlChar * search_term);
int count_items(xmlTextReaderPtr reader);