check_PROGRAMS = test_examplemod
include ../test_common.mk

test_examplemod_SOURCES = \
	runners/runner_test_examplemod.c \
	test_examplemod.c \
	../../src/examplemod/examplemod.h \
	../../src/exampletwo/mocks/mock_exampletwo.h \
	$(CMOCK_SOURCES)
test_examplemod_LDADD = \
	../../src/examplemod/libexamplemod.a \
	../../src/exampletwo/libexampletwo_mock.a
test_examplemod_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/exampletwo
