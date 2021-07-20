LIBS=`ncursesw6-config --cflags --libs` `xml2-config --cflags --libs` -lcurl -lpthread
EXECUTABLE = podcast_reader

all: $(EXECUTABLE) po/cs/$(EXECUTABLE).mo

$(EXECUTABLE): main.c fileop.c xmlop.c strop.c
	gcc -g -o $@ $^ -std=gnu18 -pedantic -Wall -Wextra $(LIBS)

po/cs/$(EXECUTABLE).mo: po/cs/$(EXECUTABLE).po
	msgfmt --output-file=$@ $<

po/cs/$(EXECUTABLE).po: po/$(EXECUTABLE).pot
	msgmerge --update $@ $<

po/$(EXECUTABLE).pot:
	xgettext --keyword=_ --language=C --from-code=utf-8 --add-comments --sort-output -o po/podcast_reader.pot `ls *.c`

clean:
	$(RM) $(EXECUTABLE) po/cs/*.mo
