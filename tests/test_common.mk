CMOCK_SOURCES = ../../third-party/CMock/src/cmock.c \
	../../third-party/CMock/src/cmock.h \
	../../third-party/CMock/src/cmock_internals.h \
	../../third-party/CMock/vendor/unity/src/unity.c \
	../../third-party/CMock/vendor/unity/src/unity.h \
	../../third-party/CMock/vendor/unity/src/unity_internals.h

AM_LDFLAGS = -pthread
AM_CPPFLAGS = \
	-I$(top_srcdir)/third-party/CMock/vendor/unity/src \
	-I$(top_srcdir)/third-party/CMock/src \
	-I$(top_srcdir)/src \
	-I../../src
if BUILD_LINUX
AM_CPPFLAGS += -DLINUX
endif
if BUILD_WINDOWS
AM_CPPFLAGS += -DWINDOWS
endif
if BUILD_APPLE
AM_CPPFLAGS += -DAPPLE
endif

TESTS = $(check_PROGRAMS)

CLEANFILES = runners

runners/runner_test_%.c: test_%.c
	@test -n "$(RUBY)" || { echo "\nPlease install Ruby to run tests.\n"; exit 1; }
	$(RUBY) $(top_srcdir)/third-party/CMock/vendor/unity/auto/generate_test_runner.rb $< $@
