# Developing m65tool

TODO:

- How to keep internal module headers private? Right now I'm adding the entire
  module subdir to the calling module's include path, which includes internal
  headers.

- How to keep internal cross-file (non-static) methods private? Are these
  exposed in the `.a`? Should they use an `_examplemod_` prefix?

- Macro-ize as much of the Makefile logic as possible! Way too much
  boilerplate, especially for tests!

- Test and document CMock.

- Test and document debugger integration with VSCode.

- Test and document Valgrind usage.

- `make format`

- Test dist on Linux.

- Instructions for building for Windows.

## Getting Started

Make sure you have the following tools installed:

- A [gcc](https://gcc.gnu.org/)-compatible C compiler
- [GNU Automake](https://www.gnu.org/software/automake/manual/html_node/index.html)
- [Ruby](https://www.ruby-lang.org/en/) 2.x or later, for unit test code
  generation

Mac users are strongly recommended to install [Homebrew](https://brew.sh/) to
manage tool installation. Installing Homebrew also installs the XCode Command
Line Tools, which provides a gcc-compatible C compiler and GNU Automake. Once
installed, run the following command to install the remaining tools:

```text
brew install ruby git gh
```

Clone the `m65tool` Github repo:

```text
gh repo clone dansanderson/m65tool
```

Run Automake's `autoreconf` to generate the initial set of build files, then
run the `./configure` script that it produces:

```text
cd m65tool
autoreconf --install
./configure
```

Finally, use `make` to build the tool for the first time:

```text
make
```

Changes to `configure.ac` or `Makefile.am` will regenerate the appropriate
files during the next `make`. If for some reason this doesn't work, you can
re-run `autoreconf` as needed.

## Modules

Source code is organized into modules. Each module is a subdirectory of `src/`
named after the module. (The `main()` for `m65tool` is in the `m65tool` module.)

```text
src/examplemod/
```

Each module must have at least one `.h` file and one
`.c` named after the module containing the declarations and definitions of all
exported functions and globals.

```text
src/examplemod/
  examplemod.c
  examplemod.h
```

Each exported function and global must begin with the name of the module
followed by an underscore. In `examplemod.h`:

```c
/**
 * @brief Add two numbers.
 * @param a first addend
 * @param b second addend
 * @returns The sum of a and b.
 **/
int examplemod_add(int a, int b);
```

In `examplemod.c`:

```c
#include "examplemod.h"

int examplemod_add(int a, int b) {
  return a + b;
}
```

A module can have additional source files private to the module.

```text
src/examplemod/
  ...
  priv.c
  priv.h
```

Each module is built to an internal `.a` library for use by other modules. In
`Makefile.am`:

```makefile
noinst_LIBRARIES = src/examplemod/libexamplemod.a
src_examplemod_libexamplemod_a_SOURCES = src/examplemod/examplemod.c src/examplemod/examplemod.h src/examplemod/priv1.c src/examplemod/priv1.h
```

To depend on a module from another module, cite the dependency's `.h` in
`SOURCES`, its `.a` in `LDADD`, and its source dir as an inclue in `CPPFLAGS`.
There is no reason to mention any of the dependency's internal source files.

```makefile
bin_PROGRAMS = src/m65tool/m65tool
src_m65tool_m65tool_SOURCES = src/m65tool/m65tool.c src/examplemod/examplemod.h
src_m65tool_m65tool_LDADD = src/examplemod/libexamplemod.a
src_m65tool_m65tool_CPPFLAGS = -I$(top_srcdir)/src/examplemod
```

## Tests

Unit tests are in the `tests/` directory, in a subdirectory named after the
module. Each source file of tests has a name that ends in `_test.c`.

```text
tests/examplemod/
  examplemod_test.c
```

Tests use the [Unity Test](https://github.com/ThrowTheSwitch/Unity) framework.
See [Unity Assertions
Reference](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md).

Each test is a `void f(void)` function whose name begins with `test_`. By
convention, the test function name has three sections: the unit of work, the
state under test, and the expected behavior. Each section uses `CapitolCasing`
and is separated by underscores.

```c
void test_ExamplemodPrintAllMessages_Always_PrintsMessages(void) {
  ...
}
```

A file of tests can contain a `void setUp(void)` and `void tearDown(void)` function, which are
called automatically by the framework before and after each test.

```c
#include "examplemod.h"

#include <stdio.h>

#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_ExamplemodPrintAllMessages_Always_PrintsMessages(void) {
  examplemod_print_all_messages();

  TEST_FAIL_MESSAGE("This is a test that fails.");
}
```

Each test has a "runner" source file that is generated at build time by this
rule, one per `_test.c` file. It depends on the module under test, as well as
Unity Test.

```makefile
tests/examplemod/examplemod_test_Runner.c:
	test -n "$(RUBY)" || { echo "\nPlease install Ruby to run tests.\n"; exit 1; }
	$(RUBY) $(top_srcdir)/third-party/Unity/auto/generate_test_runner.rb $(top_srcdir)/tests/examplemod/examplemod_test.c

tests_examplemod_examplemod_test_Runner_SOURCES = tests/examplemod/examplemod_test_Runner.c tests/examplemod/examplemod_test.c third-party/Unity/src/unity.c third-party/Unity/src/unity.h third-party/Unity/src/unity_internals.h third-party/Unity/auto/generate_test_runner.rb
tests_examplemod_examplemod_test_Runner_LDADD = src/examplemod/libexamplemod.a
tests_examplemod_examplemod_test_Runner_LDFLAGS = -pthread
tests_examplemod_examplemod_test_Runner_CPPFLAGS = -I$(top_srcdir)/third-party/Unity/src -I$(top_srcdir)/src/examplemod
```

Each generated `_Runner.c` source file must be specified under `CLEANFILES`.
Each `_Runner` program must be specified under `check_PROGRAMS`.

```makefile
CLEANFILES = tests/examplemod/examplemod_test_Runner.c
check_PROGRAMS = tests/examplemod/examplemod_test_Runner
TESTS = $(check_PROGRAMS)
```

To run all tests:

```text
make check
```

To run specific tests:

```text
make check TESTS='tests/examplemod/examplemod_test_Runner'
```

The `_Runner` programs are binary programs and can be used with a debugger.

## Adding files to the distribution

`EXTRA_DIST` in `Makefile.am` lists all files that aren't build dependencies
that should be added to the final distribution. This should include every file
in `docs/`, for example.

## Producing and testing the distribution

To make the distribution:

```text
make dist
```

This produces a file such as `m65tool-0.1.tar.gz`. This is the file someone
will download to build and install the tool from source.

You can run through the steps to build from the distribution like so:

```text
tar xzf m65tool-0.1.tar.gz
cd m65tool-0.1
./configure
make
```

Be sure to examine the distribution to make sure the appropriate non-build
files are included.

## VSCode

`.vscode/settings.json`

- `"C_Cpp.default.includePath"` must include each module source directory to
  resolve header includes.

## Style

m65tool uses standard C17. GNU extensions are disabled to increase the chances
that the code is compatible with other compilers such as Visual Studio 2019.

Code style is based on the [Google C/C++ Style
Guide](https://google.github.io/styleguide/cppguide.html). It is enforced by
`clang-format`. Especially:

- Indents are two spaces. No tabs, unless in `Makefile.am` where tabs are
  significant.

- Ordering of `#include`. (clang-format will re-order correctly.)

- Prefer `sizeof(var)` to `sizeof(type)`.

Some advice is accepted from the [GNU Coding
Standards](https://www.gnu.org/prep/standards/standards.html), especially:

- Use dynamic memory allocation for user input to avoid arbitrary limits on
  input sizes. Do so consistently and safely.

- Check every call to `malloc` and `realloc` for a NULL result. If a `malloc`
  fails in a non-interactive mode, abort the program. If it fails as a result
  of an interaction, abort the interaction and report the issue, but continue
  to accept input.

- `free` every successful `malloc`.

- Use length-restricted string functions (`strlcpy`) whenever a length is
  known. In most cases, you can perform an unbounded `strlen` on a dynamically
  allocated string returned by a trusted library (e.g. `readline`), then keep
  track of all appropriate lengths from then on. For example, to concatenate
  two dynamically allocated strings of unknown length, use `strlen` on both
  strings, `malloc` the correct amount of memory, then use `strlcat` (not
  `strcat`) to produce the result.

Naming:

- Function and global variable names use `snake_case`. Exceptions are Unity Test
  function names.

- Constant variable names use `UPPER_SNAKE_CASE`.

- Inner command names use all lowercase letters, with no delimiters between
  words. A command name should be a single word or abbreviation, if possible.

- Command-line argument names are all lowercase and use hyphen delimiters.
  Equivalent configuration property names use underscore delimiters.

Comments:

- A header file should have a file doc comment and a doc comment for each
  function and variable declaration. Use `@file`, `@brief`, `@param`,
  `@returns`. Use `/** ... */`.

Reminders of C best practices:

- Use `const` on pointer-type parameters to indicate that the memory is not
  modified. Take care to use the `const` keyword on both the pointer type and
  the pointed-to type: `void do_something(const char* const mychrp)`

  - It is not as important, but still nice, to label non-pointer-type
    parameters as `const` because they are unlikely to be modified by the
    function. ("Const correctness" is more important in C++ than C.)

- Use `static` for functions and variables private to a file.

- Prefer `const` variables for constants over `#define` where possible.

- Use `stdint.h` explicit width integer types, especially `uint8_t`.

- Use `size_t` from `stdlib.h` (or `stddef.h` or `stdio.h`) to represent byte sizes.

- Use the `offsetof(type, member)` macro from `stddef.h` to find the memory
  offset of a member of a struct.

- Use `stdbool.h` for `bool`, `true`, and `false`. Use `NULL` from `stdlib.h`
  (or `stdio.h` or `stddef.h`).
