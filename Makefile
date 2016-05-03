CFLAGS := $(CFLAGS) `pkg-config --cflags glib-2.0` `pkg-config --cflags gobject-2.0` `pkg-config --cflags x11` -std=c11 -D_DEFAULT_SOURCE

SOURCES := tray.c
LIBS := -lupower-glib `pkg-config --libs glib-2.0` `pkg-config --libs gobject-2.0` `pkg-config --libs x11`

tray: $(SOURCES)
	$(CC) $(CFLAGS) $< $(LIBS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f *.o tray
