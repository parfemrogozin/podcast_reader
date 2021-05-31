LIBS=`ncursesw6-config --cflags --libs` `xml2-config --cflags --libs` -lcurl -lpthread
EXECUTABLE = podcasty

$(EXECUTABLE): main.c fileop.c xmlop.c strop.c
	gcc -g -o $@ $^ -std=gnu18 -pedantic -Wall -Wextra $(LIBS)

clean:
	$(RM) $(EXECUTABLE)
