include ../module_common.mk

noinst_LIBRARIES = libexamplemod.a
libexamplemod_a_SOURCES = \
	examplemod.c \
	examplemod.h \
	priv1.c \
	priv1.h \
	../exampletwo/exampletwo.h

check_LIBRARIES = libexamplemod_mock.a
libexamplemod_mock_a_SOURCES = \
	mocks/mock_examplemod.c \
	mocks/mock_examplemod.h
libexamplemod_mock_a_CPPFLAGS = $(MOCK_CPPFLAGS)
