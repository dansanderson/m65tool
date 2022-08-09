include ../module_common.mk

noinst_LIBRARIES = libexampletwo.a
libexampletwo_a_SOURCES = \
	exampletwo.c \
	exampletwo.h

check_LIBRARIES = libexampletwo_mock.a
libexampletwo_mock_a_SOURCES = \
	mocks/mock_exampletwo.c \
	mocks/mock_exampletwo.h
libexampletwo_mock_a_CPPFLAGS = $(MOCK_CPPFLAGS) -I$(top_srcdir)/src/exampletwo
