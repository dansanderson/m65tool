AM_CPPFLAGS = -I$(top_srcdir)/src
if BUILD_LINUX
AM_CPPFLAGS += -DLINUX
endif
if BUILD_WINDOWS
AM_CPPFLAGS += -DWINDOWS
endif
if BUILD_APPLE
AM_CPPFLAGS += -DAPPLE
endif

# Due to a limitation in how CMock #includes the module header from the mock,
# we either need to generate the mock in the same directory (MOCK_OUT=.) or use
# an undocumented config option (orig_header_include_fmt) to force it to use a
# relative path. This opts for the former option.
mock_%.c: %.h
	test -n "$(RUBY)" || { echo "\nPlease install Ruby to run tests.\n"; exit 1; }
	CMOCK_DIR=$(top_srcdir)/third-party/CMock \
	MOCK_OUT=. \
	$(RUBY) $(top_srcdir)/third-party/CMock/scripts/create_mock.rb $(top_srcdir)/src/$(subst .c,,$(subst mock_,,$@))/$(subst .c,,$(subst mock_,,$@)).h

mock_%.h: mock_%.c

MOCK_CPPFLAGS = \
	-I$(top_srcdir)/third-party/CMock/vendor/unity/src \
	-I$(top_srcdir)/third-party/CMock/src \
	-I$(top_srcdir)/src
