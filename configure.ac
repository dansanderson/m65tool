AC_INIT([m65tool], [0.1])
AM_INIT_AUTOMAKE([foreign -Wall -Werror -Wno-portability subdir-objects])

AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
AC_PATH_PROG([RUBY], [ruby])

CFLAGS="$CFLAGS -std=c17"

# https://www.gnu.org/software/automake/manual/html_node/Automake-Silent-Rules.html
# AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES(
  [Makefile]
  [src/Makefile]
  [src/examplemod/Makefile]
  [src/exampletwo/Makefile]
  [src/m65tool/Makefile]
  [tests/Makefile]
  [tests/examplemod/Makefile])

AC_OUTPUT
