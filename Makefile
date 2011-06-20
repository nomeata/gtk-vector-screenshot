all: libpdfscreenshot.so module.so

libpdfscreenshot.so: pdfscreenshot.c
	gcc -shared -fPIC -o libpdfscreenshot.so pdfscreenshot.c -ldl `pkg-config --cflags gdk-2.0`


module.so: module.c
	gcc -std=gnu99 -fPIC -shared $$(pkg-config --cflags --libs gtk+-3.0) $< -o $@
