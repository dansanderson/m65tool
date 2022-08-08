AC_INIT([m65tool], [0.1])
AM_INIT_AUTOMAKE([foreign -Wall -Werror subdir-objects])

AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
AC_PATH_PROG([RUBY], [ruby])

CFLAGS="$CFLAGS -std=c17"

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES(
  [Makefile]
  [src/Makefile]
  [src/examplemod/Makefile]
  [src/m65tool/Makefile]
  [tests/Makefile]
  [tests/examplemod/Makefile])

AC_OUTPUT
