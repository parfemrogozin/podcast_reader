#include <libxml/xmlreader.h>
#include <string.h>
#include "fileop.h"
#include "xmlop.h"

const xmlChar * get_enclosure(xmlTextReaderPtr reader, int position)
{
  const xmlChar * url = NULL;
  const xmlChar * search_tag = (const xmlChar *)"enclosure";
  const xmlChar * search_attribute = (const xmlChar *)"url";
  const int target_depth = 3;
  int ret, depth, type;
  int count = 0;
  const xmlChar * tag_name;

  ret = xmlTextReaderRead(reader);

  while (ret == 1)
  {
    ret = xmlTextReaderRead(reader);
    depth = xmlTextReaderDepth(reader);
    if (depth != target_depth) continue;
    type = xmlTextReaderNodeType(reader);
    if (type != 1) continue;
    tag_name = xmlTextReaderConstName(reader);
    if (!xmlStrcmp(tag_name, search_tag))
    {
      ++count;
      if (count == position)
      {
        url =  xmlTextReaderGetAttribute(reader, search_attribute);
        break;
      }
    }
  }
  return url;
}

const xmlChar * get_description(xmlTextReaderPtr reader, int position)
{
  const xmlChar * description_text = NULL;
  const xmlChar * search_tag = (const xmlChar *)"description";
  const int target_depth = 3;
  int ret, depth, type;
  int count = 0;
  int found = 0;
  const xmlChar * tag_name;

  ret = xmlTextReaderRead(reader);

  while (ret == 1)
  {
    ret = xmlTextReaderRead(reader);
    if (found)
    {
      description_text = xmlTextReaderConstValue(reader);
      break;
    }
    depth = xmlTextReaderDepth(reader);
    if (depth != target_depth) continue;
    type = xmlTextReaderNodeType(reader);
    if (type != 1) continue;
    tag_name = xmlTextReaderConstName(reader);
    if (!xmlStrcmp(tag_name, search_tag))
    {
      ++count;
      if (count == position)
      {
        found = 1;
      }
    }
  }
  return description_text;
}

int read_feed(xmlTextReaderPtr reader, char * menu_items)
{
  const xmlChar * item = (const xmlChar *)"item";
  const xmlChar * title = (const xmlChar *)"title";
  int ret;
  int depth;
  int type;
  const xmlChar * tag_name = NULL;
  const int min_depth = 2;
  int inside = 0;
  int is_title = 0;
  int i = 0;


  ret = xmlTextReaderRead(reader);
  if (ret != 1)
  {
    xmlFreeTextReader(reader);
    return -1;
  }
  while (ret == 1)
  {
    ret = xmlTextReaderRead(reader);
    depth = xmlTextReaderDepth(reader);
    if (depth < min_depth)
    {
      continue;
    }
    type = xmlTextReaderNodeType(reader);
    if (inside == 0 && type == 1)
    {
      tag_name = xmlTextReaderConstName(reader);
      if (!xmlStrcmp(tag_name, item))
      {
        inside = 1;
        continue;
      }
    }
    if (inside == 1 && type == 1)
    {
      tag_name = xmlTextReaderConstName(reader);
      if (!xmlStrcmp(tag_name, title))
      {
        is_title = 1;
        continue;
      }
    }
    if (is_title == 1 && type == 3)
    {
      strncpy (menu_items + ITEMSIZE * i, (char *) xmlTextReaderConstValue(reader), ITEMSIZE - 1);
      ++i;
      is_title = 0;
    }
  }
  xmlFreeTextReader(reader);
  return 0;
}

const xmlChar * read_single_value(xmlTextReaderPtr reader, const xmlChar * search_term)
{
  int ret;
  int type;
  int depth;
  const int min_depth = 2;
  const xmlChar * tag_name = NULL;
  const xmlChar * value = NULL;

  do
  {
    ret = xmlTextReaderRead(reader);
    depth = xmlTextReaderDepth(reader);
    if (depth >= min_depth)
    {
      type = xmlTextReaderNodeType(reader);
      if (type == 1)
      {
        tag_name = xmlTextReaderConstName(reader);
      }
      else if (type == 15)
      {
        tag_name = NULL;
      }
      else if ((type == 3 || type == 4) && !xmlStrcmp(tag_name, search_term))
      {
        value = xmlTextReaderConstValue(reader);
        break;
      }
    }
  }
  while (ret == 1);

  return value;
}

int count_items(xmlTextReaderPtr reader)
{
  int item_count = 0;
  const xmlChar * search_term = (const xmlChar *)"item";
  const xmlChar * tag_name = NULL;
  int depth;
  int type;
  const int min_depth = 2;
  int ret;

  do
  {
    ret = xmlTextReaderRead(reader);
    depth = xmlTextReaderDepth(reader);
    if (depth < min_depth)
    {
      continue;
    }
    type = xmlTextReaderNodeType(reader);
    if (type == 1)
    {
      tag_name = xmlTextReaderConstName(reader);
      if (!xmlStrcmp(tag_name, search_term))
      {
        ++item_count;
      }

    }
  }
  while (ret == 1);
  xmlFreeTextReader(reader);
  return item_count;
}

