SUBDIRS = src tests

## Internal modules

#noinst_LIBRARIES = build/modules/libexamplemod.a
#build_modules_libexamplemod_a_SOURCES = \
#	src/examplemod/examplemod.c \
#	src/examplemod/examplemod.h \
#	src/examplemod/priv1.c \
#	src/examplemod/priv1.h


## The m65tool program

#bin_PROGRAMS = build/m65tool
#build_m65tool_SOURCES = \
#	src/m65tool/m65tool.c \
#	src/examplemod/examplemod.h
#build_m65tool_LDADD = build/modules/libexamplemod.a
#build_m65tool_CPPFLAGS = -I$(top_srcdir)/src


## Tests

#check_PROGRAMS = build/test/test_examplemod

#build/test/runners/runner_test_examplemod.c: tests/examplemod/test_examplemod.c
#	test -n "$(RUBY)" || { echo "\nPlease install Ruby to run tests.\n"; exit 1; }
#	$(RUBY) $(top_srcdir)/third-party/CMock/vendor/unity/auto/generate_test_runner.rb \
#		$(top_srcdir)/tests/examplemod/test_examplemod.c \
#		build/test/runners/runner_test_examplemod.c

#build_test_test_examplemod_SOURCES = \
#	build/test/runners/runner_test_examplemod.c \
#	tests/examplemod/test_examplemod.c \
#	third-party/CMock/src/cmock.c \
#	third-party/CMock/src/cmock.h \
#	third-party/CMock/src/cmock_internals.h \
#	third-party/CMock/vendor/unity/src/unity.c \
#	third-party/CMock/vendor/unity/src/unity.h \
#	third-party/CMock/vendor/unity/src/unity_internals.h \
#	src/examplemod/examplemod.h
#build_test_test_examplemod_LDADD = src/examplemod/libexamplemod.a
#build_test_test_examplemod_LDFLAGS = -pthread
#build_test_test_examplemod_CPPFLAGS = \
#	-I$(builddir)/build/test/mocks \
#	-I$(top_srcdir)/third-party/CMock/vendor/unity/src \
#	-I$(top_srcdir)/third-party/CMock/src \
#	-I$(top_srcdir)/src

#build/test/test_examplemod.testresult: build/test/test_examplemod
#	-./build/test/test_examplemod > ./build/test/test_examplemod.testresult 2>&1

#test_summary: build/test/test_examplemod.testresult
#	@UNITY_DIR=$(top_srcdir)/third-party/CMock/vendor/unity ruby $(top_srcdir)/third-party/CMock/scripts/test_summary.rb

#TESTS = $(check_PROGRAMS)

#CLEANFILES = build/test


## Non-source dist

EXTRA_DIST = \
	docs/developing.md \
	third-party/CMock/LICENSE.txt \
  third-party/CMock/scripts/test_summary.rb \
  third-party/CMock/vendor/unity/auto/generate_test_runner.rb \
  third-party/CMock/vendor/unity/auto/unity_test_summary.rb
