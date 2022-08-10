AC_INIT([m65tool], [0.1])
AM_INIT_AUTOMAKE([foreign -Wall -Werror -Wno-portability subdir-objects])

# Detect target OS (including cross-compile)
# Thanks to https://tversteeg.nl/posts/makefile-os/
AC_CANONICAL_HOST
build_linux=no
build_windows=no
build_apple=no
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    cygwin*|mingw*)
        build_windows=yes
        ;;
    darwin*)
        build_apple=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac
AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_APPLE], [test "$build_apple" = "yes"])

AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
AC_PATH_PROG([RUBY], [ruby])
AM_PATH_PYTHON([3.0],,[:])
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])
AC_PATH_PROG([CLANG_FORMAT], [clang-format])

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
