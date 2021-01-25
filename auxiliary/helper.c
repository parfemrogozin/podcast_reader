static void
processNode(xmlTextReaderPtr reader) {
    const xmlChar *name, *value;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
  name = BAD_CAST "--";

    value = xmlTextReaderConstValue(reader);

    printf("%d %d %s %d %d",
      xmlTextReaderDepth(reader),
      xmlTextReaderNodeType(reader),
      name,
      xmlTextReaderIsEmptyElement(reader),
      xmlTextReaderHasValue(reader));
    if (value == NULL)
  printf("\n");
    else {
        if (xmlStrlen(value) > 40)
            printf(" %.40s...\n", value);
        else
      printf(" %s\n", value);
    }
}

/*
  int ret;
  xmlTextReaderPtr reader = xmlReaderForFile(file_list[0], NULL,0);
  if (reader != NULL)
  {
    ret = xmlTextReaderRead(reader);
    while (ret == 1)
    {
      processNode(reader);
      ret = xmlTextReaderRead(reader);
    }
  }
  xmlFreeTextReader(reader);
*/
