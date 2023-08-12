LDLIBS = `ncursesw6-config --libs` `xml2-config --libs` `curl-config --libs` -lpthread -lrt
CFLAGS = `ncursesw6-config --cflags` `xml2-config --cflags` `curl-config --cflags`

ifeq ($(PREFIX),)
	PREFIX := $$HOME
endif

all: podcast_reader po/cs/podcast_reader.mo

podcast_reader: gui.c fileop.c xmlop.c strop.c id3op.c

po/cs/podcast_reader.mo: po/cs/podcast_reader.po
	msgfmt --output-file=$@ $<

po/cs/podcast_reader.po: po/podcast_reader.pot
	msgmerge --update $@ $<

po/podcast_reader.pot:
	xgettext --keyword=_ --language=C --from-code=utf-8 --add-comments --sort-output -o po/.pot `ls *.c`

install:
	install -d $(PREFIX)/.local/bin
	install podcast_reader $(PREFIX)/.local/bin
	install -d $(PREFIX)/.local/share/locale/cs/LC_MESSAGES
	install -m 644 po/cs/podcast_reader.mo $(PREFIX)/.local/share/locale/cs/LC_MESSAGES
	install -m 600 rss_feed_list.txt $(PREFIX)/.config

uninstall:
	$(RM) $(PREFIX)/.local/bin/podcast_reader
	$(RM) $(PREFIX)/.local/share/locale/cs/LC_MESSAGES/podcast_reader.mo

clean:
	$(RM) podcast_reader po/cs/*.mo
