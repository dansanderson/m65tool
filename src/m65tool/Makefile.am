AM_CPPFLAGS = -I$(top_srcdir)/src

m65tool_SOURCES = \
	m65tool.c \
	../examplemod/examplemod.h
m65tool_LDADD = \
	../examplemod/libexamplemod.a \
	../exampletwo/libexampletwo.a

if BUILD_WINDOWS
bin_PROGRAMS = m65tool.exe
m65tool_exe_SOURCES = $(m65tool_SOURCES)
m65tool_exe_LDADD = $(m65tool_LDADD)
else
bin_PROGRAMS = m65tool
endif
