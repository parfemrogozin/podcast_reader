LIBS=`ncursesw6-config --cflags --libs` `xml2-config --cflags --libs` -lcurl -lpthread
EXECUTABLE = podcasty

$(EXECUTABLE): main.c fileop.c
	gcc -g -o $@ $^ -std=c18 -pedantic -Wall -Wextra $(LIBS)

clean:
	$(RM) $(EXECUTABLE)
