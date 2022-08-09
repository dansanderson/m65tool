AM_CPPFLAGS = -I$(top_srcdir)/src

mocks/mock_%.c: %.h
	test -n "$(RUBY)" || { echo "\nPlease install Ruby to run tests.\n"; exit 1; }
	CMOCK_DIR=$(top_srcdir)/third-party/CMock \
	MOCK_OUT=mocks \
	$(RUBY) $(top_srcdir)/third-party/CMock/scripts/create_mock.rb $(top_srcdir)/src/$(subst .c,,$(subst mocks/mock_,,$@))/$(subst .c,,$(subst mocks/mock_,,$@)).h

mocks/mock_%.h: mocks/mock_%.c

MOCK_CPPFLAGS = \
	-I$(top_srcdir)/third-party/CMock/vendor/unity/src \
	-I$(top_srcdir)/third-party/CMock/src \
	-I$(top_srcdir)/src

CLEANFILES = mocks
