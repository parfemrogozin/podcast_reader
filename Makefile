LIBS=`ncursesw6-config --cflags --libs` `xml2-config --cflags --libs` -lcurl -lpthread -lrt
EXECUTABLE = podcast_reader

ifeq ($(PREFIX),)
	PREFIX := $$HOME
endif

all: $(EXECUTABLE) po/cs/$(EXECUTABLE).mo

$(EXECUTABLE): main.c fileop.c xmlop.c strop.c id3op.c
	gcc -g -o $@ $^ -std=gnu18 -pedantic -Wall -Wextra $(LIBS)

po/cs/$(EXECUTABLE).mo: po/cs/$(EXECUTABLE).po
	msgfmt --output-file=$@ $<

po/cs/$(EXECUTABLE).po: po/$(EXECUTABLE).pot
	msgmerge --update $@ $<

po/$(EXECUTABLE).pot:
	xgettext --keyword=_ --language=C --from-code=utf-8 --add-comments --sort-output -o po/podcast_reader.pot `ls *.c`

install:
	install -d $(PREFIX)/.local/bin
	install $(EXECUTABLE) $(PREFIX)/.local/bin
	install -d $(PREFIX)/.local/share/locale/cs/LC_MESSAGES
	install -m 644 po/cs/$(EXECUTABLE).mo $(PREFIX)/.local/share/locale/cs/LC_MESSAGES
	install -m 600 rss_feed_list.txt $(PREFIX)/.config

uninstall:
	$(RM) $(PREFIX)/.local/bin/$(EXECUTABLE)
	$(RM) $(PREFIX)/.local/share/locale/cs/LC_MESSAGES/$(EXECUTABLE).mo

clean:
	$(RM) $(EXECUTABLE) po/cs/*.mo
