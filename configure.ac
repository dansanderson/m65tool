m4_include([m4/libuv-check-flags.m4])
AC_INIT([m65tool], [0.1])
AM_INIT_AUTOMAKE([foreign -Wall -Werror -Wno-portability subdir-objects])

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
    msys*)
        AC_MSG_ERROR(["Use a MSYS2 MinGW terminal (and not the MSYS2 MSYS terminal) to build a Windows .exe"])
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac
AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_APPLE], [test "$build_apple" = "yes"])

AC_PROG_CC
CC_CHECK_CFLAGS_APPEND([-std=c17])
CC_CHECK_CFLAGS_APPEND([-Wall])
CC_CHECK_CFLAGS_APPEND([-Wextra])

AM_PROG_AR
AC_PATH_PROG([RUBY], [ruby])
LT_INIT
AM_PATH_PYTHON([3.0],,[:])
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])
AC_PATH_PROG([CLANG_FORMAT], [clang-format])

AX_CODE_COVERAGE

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
