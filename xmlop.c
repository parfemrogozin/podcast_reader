#include <libxml/xmlreader.h>
#include <string.h>
#include <stdbool.h>

#include "fileop.h"
#include "xmlop.h"

static bool is_node(xmlTextReaderPtr reader, const xmlChar * reader_interned_string, const int level)
{
  bool found = false;

  if ( xmlTextReaderDepth(reader) == level)
  {
    if ( XML_READER_TYPE_ELEMENT == xmlTextReaderNodeType(reader) )
    {
      if ( xmlTextReaderConstName(reader) == reader_interned_string )
      {
        found = true;
      }
    }
  }

  return found;
}

unsigned int count_nodes(const char * file_name, const char * tag_name, const int level)
{
  xmlTextReaderPtr reader = xmlReaderForFile(file_name, NULL,0);
  const xmlChar * search_term = xmlTextReaderConstString(reader, (const xmlChar *)tag_name);
  unsigned int item_count = 0;
  int ret = 0;

  do
  {
    ret = xmlTextReaderRead(reader);
    if ( is_node(reader, search_term, level) )
    {
      item_count++;
    }
  }
  while (ret > 0);
  xmlFreeTextReader(reader);
  return item_count;
}

void copy_single_content(char * rss_file, const int min_depth, const char * enclosing_tag, unsigned int position, char * free_slot, int free_size)
{
  xmlTextReaderPtr reader = xmlReaderForFile(rss_file, NULL,0);
  const xmlChar * search_term = xmlTextReaderConstString(reader, (const xmlChar *)enclosing_tag);
  unsigned int count = 0;
  bool inside_tag = false;
  int ret = 0;

  do
  {
    ret = xmlTextReaderRead(reader);

    if ( inside_tag )
    {
      strncpy(free_slot, (char *) xmlTextReaderConstValue(reader), free_size);
      break;
    }

    if ( is_node(reader, search_term, min_depth) )
    {
      count++;
      if (count == position)
      {
        inside_tag = true;
      }
    }

  }
  while (ret > 0);
  xmlFreeTextReader(reader);
}


char * get_enclosure(char * rss_file, const unsigned int position)
{
  xmlTextReaderPtr reader = xmlReaderForFile(rss_file, NULL,0);
  const xmlChar * search_term = xmlTextReaderConstString(reader, (const xmlChar *)"enclosure");
  unsigned int count = 0;
  int ret = 0;

  do
  {
    ret = xmlTextReaderRead(reader);
    if ( is_node(reader, search_term, 3) )
    {
      count++;
      if (count == position)
      {
        return (char *) xmlTextReaderGetAttribute(reader, (const xmlChar *)"url");
      }
    }
  }
  while (ret > 0);
  xmlFreeTextReader(reader);
  return NULL;
}



void read_feed(char * rss_file, char * menu_items)
{
  xmlTextReaderPtr reader = xmlReaderForFile(rss_file, NULL,0);
  const xmlChar * parent = xmlTextReaderConstString(reader, (const xmlChar *)"item");
  const xmlChar * search_term = xmlTextReaderConstString(reader, (const xmlChar *)"title");
  int i = 0;
  bool inside_parent = false;
  bool inside_tag = false;
  int ret = 0;

  do
  {
    ret = xmlTextReaderRead(reader);

    if ( inside_tag )
    {
      strncpy (menu_items + ITEMSIZE * i, (char *) xmlTextReaderConstValue(reader), ITEMSIZE - 1);
      ++i;
      inside_tag = false;
      inside_parent = false;
    }

    else if ( is_node(reader, parent, 2) )
    {
      inside_parent = true;
    }
    else if ( inside_parent && is_node(reader, search_term, 3) )
    {
      inside_tag = true;
    }


  }
  while (ret > 0);
  xmlFreeTextReader(reader);
}