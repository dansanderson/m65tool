include ../module_common.mk

noinst_LIBRARIES = libexampletwo.a
libexampletwo_a_SOURCES = \
	exampletwo.c \
	exampletwo.h

check_LIBRARIES = libexampletwo_mock.a
libexampletwo_mock_a_SOURCES = \
	mock_exampletwo.c \
	mock_exampletwo.h
libexampletwo_mock_a_CPPFLAGS = $(MOCK_CPPFLAGS) -I$(top_srcdir)/src/exampletwo

CLEANFILES = mock_exampletwo.c mock_exampletwo.h
