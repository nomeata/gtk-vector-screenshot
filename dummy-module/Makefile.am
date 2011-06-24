moduledir = $(libdir)/gtk-2.0/modules

libgtk_vector_screenshot_la_SOURCES = dummy-module.c

libgtk_vector_screenshot_la_LDFLAGS = \
	-rpath $(moduledir) -module -avoid-version -export-dynamic\
	$(GTK2_LIBS) \
	@LDFLAGS@

libgtk_vector_screenshot_la_LIBADD =

module_LTLIBRARIES = libgtk-vector-screenshot.la

INCLUDES = \
	$(GTK2_CFLAGS)
