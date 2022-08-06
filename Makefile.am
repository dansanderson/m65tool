## Internal modules

noinst_LIBRARIES = src/examplemod/libexamplemod.a
src_examplemod_libexamplemod_a_SOURCES = src/examplemod/examplemod.c src/examplemod/examplemod.h src/examplemod/priv1.c src/examplemod/priv1.h


## The m65tool program

bin_PROGRAMS = src/m65tool/m65tool
src_m65tool_m65tool_SOURCES = src/m65tool/m65tool.c src/examplemod/examplemod.h
src_m65tool_m65tool_LDADD = src/examplemod/libexamplemod.a
src_m65tool_m65tool_CPPFLAGS = -I$(top_srcdir)/src/examplemod


## Tests

tests/examplemod/examplemod_test_Runner.c:
	test -n "$(RUBY)" || { echo "\nPlease install Ruby to run tests.\n"; exit 1; }
	$(RUBY) $(top_srcdir)/third-party/Unity/auto/generate_test_runner.rb $(top_srcdir)/tests/examplemod/examplemod_test.c

CLEANFILES = tests/examplemod/examplemod_test_Runner.c

check_PROGRAMS = tests/examplemod/examplemod_test_Runner
TESTS = $(check_PROGRAMS)

tests_examplemod_examplemod_test_Runner_SOURCES = tests/examplemod/examplemod_test_Runner.c tests/examplemod/examplemod_test.c third-party/Unity/src/unity.c third-party/Unity/src/unity.h third-party/Unity/src/unity_internals.h third-party/Unity/auto/generate_test_runner.rb
tests_examplemod_examplemod_test_Runner_LDADD = src/examplemod/libexamplemod.a
tests_examplemod_examplemod_test_Runner_LDFLAGS = -pthread
tests_examplemod_examplemod_test_Runner_CPPFLAGS = -I$(top_srcdir)/third-party/Unity/src -I$(top_srcdir)/src/examplemod


## Non-source dist

EXTRA_DIST = docs/testdoc.md third-party/CMock/LICENSE.txt third-party/Unity/LICENSE.txt
