SUBDIRS = src tests

EXTRA_DIST = \
  README.md \
	docs \
	third-party/CMock/LICENSE.txt \
  third-party/CMock/scripts/test_summary.rb \
  third-party/CMock/vendor/unity/auto/generate_test_runner.rb \
  third-party/CMock/vendor/unity/auto/unity_test_summary.rb

.PHONY: superclean format

superclean:
	@test -e "$(PYTHON)" || { echo "\nPlease install Python 3.x to run superclean.\n"; exit 1; }
	$(PYTHON) scripts/superclean.py

format:
	@test -n "$(CLANG_FORMAT)" || { echo "\nPlease install clang-format to run format.\n"; exit 1; }
	find . -type f \( -path './src/*' -o -path './tests/*' \) \( -iname '*.h' -o -iname '*.c' \) | xargs $(CLANG_FORMAT) --style=file -i

# These are made outside of make by scripts/build.py, but we include them in
# the clean.
CLEANFILES = bin/m65tool bin/m65tool.exe
