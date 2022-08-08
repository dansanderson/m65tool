check_PROGRAMS = test_examplemod

runners/runner_test_examplemod.c: test_examplemod.c
	test -n "$(RUBY)" || { echo "\nPlease install Ruby to run tests.\n"; exit 1; }
	$(RUBY) $(top_srcdir)/third-party/CMock/vendor/unity/auto/generate_test_runner.rb \
		$(top_srcdir)/tests/examplemod/test_examplemod.c \
		runners/runner_test_examplemod.c

test_examplemod_SOURCES = \
	runners/runner_test_examplemod.c \
	test_examplemod.c \
	../../third-party/CMock/src/cmock.c \
	../../third-party/CMock/src/cmock.h \
	../../third-party/CMock/src/cmock_internals.h \
	../../third-party/CMock/vendor/unity/src/unity.c \
	../../third-party/CMock/vendor/unity/src/unity.h \
	../../third-party/CMock/vendor/unity/src/unity_internals.h \
	../../src/examplemod/examplemod.h
test_examplemod_LDADD = ../../src/examplemod/libexamplemod.a
test_examplemod_LDFLAGS = -pthread
test_examplemod_CPPFLAGS = \
	-I$(builddir)/build/test/mocks \
	-I$(top_srcdir)/third-party/CMock/vendor/unity/src \
	-I$(top_srcdir)/third-party/CMock/src \
	-I$(top_srcdir)/src

test_examplemod.testresult: test_examplemod
	-./test_examplemod > test_examplemod.testresult 2>&1

test_summary: test_examplemod.testresult
	@UNITY_DIR=$(top_srcdir)/third-party/CMock/vendor/unity ruby $(top_srcdir)/third-party/CMock/scripts/test_summary.rb

TESTS = $(check_PROGRAMS)

CLEANFILES = runners
