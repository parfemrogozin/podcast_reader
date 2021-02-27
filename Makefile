LIBS=`ncursesw6-config --cflags --libs` `xml2-config --cflags --libs` -lcurl -lpthread
OUTPUT=podcasty



all:
	gcc -g -o $(OUTPUT) main.c fileop.c -std=c18 -pedantic -Wall -Wextra $(LIBS)

clean:
	$(RM) $(OUTPUT)
