moduledir = $(libdir)/gtk-3.0/modules

libgtk_vector_screenshot_la_SOURCES = gtk-vector-screenshot.c

libgtk_vector_screenshot_la_LDFLAGS = \
	-rpath $(moduledir) -module -avoid-version \
	$(GTK_LIBS) \
	@LDFLAGS@

libgtk_vector_screenshot_la_LIBADD =

module_LTLIBRARIES = libgtk-vector-screenshot.la

take_vector_screenshot_SOURCES = take-vector-screenshot.c

take_vector_screenshot_LDFLAGS = \
	$(GTK_LIBS) \
	@LDFLAGS@

bin_PROGRAMS = take-vector-screenshot

INCLUDES = \
	$(GTK_CFLAGS)
