AC_INIT(gtk-vector-screenshot, 0.1, mail@joachim-breitner.de)
AC_PREREQ(2.50)
AC_COPYRIGHT([Copyright 2011 Joachim Breitner])


dnl ################################################################
dnl # Initialize automake
dnl ################################################################
VERSION=0.1
PACKAGE=gtk-vector-screenshot

AM_INIT_AUTOMAKE([foreign])


dnl ################################################################
dnl # Specify a header configuration file
dnl ################################################################
AM_CONFIG_HEADER(config.h)


dnl ################################################################
dnl # Check for some standard stuff.
dnl ################################################################
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_INSTALL
AM_PROG_LIBTOOL

AC_C_CONST
AC_TYPE_SIZE_T
AC_EXEEXT

PKG_CHECK_MODULES(GTK, [gtk+-3.0 >= 2.90])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

CFLAGS="$CFLAGS -std=gnu99"

dnl # Use wall if we have GCC
if test "x$GCC" = "xyes"; then
   CFLAGS="$CFLAGS -Wall"
fi

AC_SUBST(CFLAGS)

dnl ################################################################
dnl # Output the Makefiles
dnl ################################################################
AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
