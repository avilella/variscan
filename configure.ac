dnl Process this file with autoconf to produce a configure script.
#AC_INIT(src/variscan.c)
#AM_INIT_AUTOMAKE(variscan, 2.0.3)
AC_INIT([variscan], [2.0.3])
AC_CONFIG_SRCDIR([src/variscan.c])
AM_INIT_AUTOMAKE

AC_ARG_ENABLE(debug,
[  --enable-debug    Turn on debugging],
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],[debug=false])

AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

dnl Checks for programs.
AC_PROG_CC

AC_OUTPUT(Makefile src/Makefile)
