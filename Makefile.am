moduledir = $(libdir)/gtk-3.0/modules

libgtk_vector_screenshot_la_SOURCES = gtk-vector-screenshot.c

libgtk_vector_screenshot_la_LDFLAGS = \
	-rpath $(moduledir) -module -avoid-version -export-dynamic\
	$(GTK_LIBS) \
	$(X_LIBS) \
	@LDFLAGS@

libgtk_vector_screenshot_la_LIBADD =

module_LTLIBRARIES = libgtk-vector-screenshot.la

take_vector_screenshot_SOURCES = take-vector-screenshot.c

take_vector_screenshot_LDFLAGS = \
	$(GTK_LIBS) \
	@LDFLAGS@

bin_PROGRAMS = take-vector-screenshot

INCLUDES = \
	$(GTK_CFLAGS) \
	$(X_CFLAGS)

take_screenshotdir  = $(datadir)/applications
dist_take_screenshot_DATA = take-vector-screenshot.desktop

EXTRA_DIST = 52load-gtk-vector-screenshot-gtk-module
