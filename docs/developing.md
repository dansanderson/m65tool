# Developing m65tool

These are instructions for developing m65tool. If you just want to build
m65tool from source for yourself, you can download the source distribution. See
README.md.

## Prerequisites

Make sure you have the following support tools installed:

- A [gcc](https://gcc.gnu.org/)-compatible C compiler
- [GNU Automake](https://www.gnu.org/software/automake/manual/html_node/index.html)
- [Ruby](https://www.ruby-lang.org/en/) 2.x or later, for unit test code
  generation

Linux can usually get these via `apt-get`. Check your OS's software packaging
system for details. On Ubuntu:

```text
sudo apt-get update
sudo apt-get install build-essential autoconf ruby-full git clang-format
```

Mac users are strongly recommended to install [Homebrew](https://brew.sh/) to
manage tool installation. Installing Homebrew also installs the XCode Command
Line Tools, which provides a gcc-compatible C compiler and GNU Automake. Once
installed, run the following command to install the remaining tools:

```text
brew install ruby git clang-format
```

## Checking out and building from the repo

Clone the `m65tool` Github repo, with submodules:

```text
git clone https://github.com/dansanderson/m65tool --recurse-submodules
```

Run Automake's `autoreconf` to generate the initial set of build files, then
run the `./configure` script that it produces:

```text
cd m65tool
autoreconf --install
./configure
```

Use `make` to build the tool. The `m65tool` binary appears in the
`./src/m65tool` directory.

```text
make
```

Use `make check` to build and run all unit tests and report a failure summary.
This builds a standalone binary for each module, such as `./tests/

```text
make check
```

Use `make distcheck` to run all tests and produce the source distribution.

```text
make distcheck
```

It is usually sufficient to run just `make` and `make check` during
development. If you make changes to `configure.ac` or `Makefile.am`, running
`make` _may or may not_ regenerate the appropriate files. If it seems like a
change to a `Makefile.am` is not having an effect, re-run
`autoreconf --install` and `./configure`.

### Wrangling the intermediate files

You probably noticed that `autoreconf`, `./configure`, `make`, and `make check`
produce dozens of intermediate files strewn about all of the source
directories. It is usually safe to ignore them. Git does: all intermediate
files are mentioned in `.gitignore`, so they don't appear as changed and won't
be committed to the repo.

There are several cleaning targets included by Autotools:

- `make clean` deletes build artifacts, just enough to inspire a subsequent
  `make` to re-build most things.

- `make distclean` also deletes some output by `./configure`. You must re-run
  `./configure` before you can `make` again.

- `make maintainer-clean` deletes a few more intermediate files. By design, it
  leaves enough files for `./configure && make` to succeed.

I found it was important to fully restore the project directory to its Git repo
state to reset artifacts from erroneous Makefile rules. To make this easier, I
have a script, `superclean.py`, in the root directory that deletes all files
explicitly ignored by Git, and deletes empty directories. You must re-run
`autoreconf --install && ./configure && make` to build again.

```text
python3 superclean.py --dry-run

python3 superclean.py
autoreconf --install
./configure
make
```

There's a make target for `superclean.py`. Note that this will not function if
there is a syntax error in the Makefile, which might be the condition you're
attempting to clean up by running `superclean.py` in the first place.

```text
make superclean
```

### Building to a different directory

This behavior of creating build files in the source tree is the default for GNU
Autotools when run from the project root directory. In theory, you can generate
the build tree in another directory, like so:

```text
mkdir build
cd build
../configure
make
```

As of this writing, I have a bug in my test Makefile logic that prevents tests
from running this way. It's probably fixable: it's an easy mistake to refer to
a source path when you mean to refer to a build path. I wasn't motivated to fix
it because a few files still get generated in the source tree regardless.

### Quieter builds

The build produces a lot of console output. This is the default for GNU
Autotools, and I have left it this way for now for troubleshooting.

To tell it to produce less build output, give `--enable-silent-rules` to
`./configure`:

```text
./configure --enable-silent-rules
```

You can silence `make` further with `--no-print-directory`:

```text
make --no-print-directory
```

I have not yet set up [silent
rules](https://www.gnu.org/software/automake/manual/html_node/Automake-Silent-Rules.html)
for things like test code generators.

To make `--enable-silent-rules` the default in `configure.ac`, uncomment this
line:

```text
AM_SILENT_RULES([yes])
```

## Modules

The source code of m65tool is organized into _modules._ Each module gets a
directory under `src/`, a directory under `tests/`, a header of exported
functions and globals, a "convenience library" (`.a`) build target, and a test
mock library build target.

Each subdirectory gets a Makefile. An effort was made to minimize module
boilerplate. Inevitably there are several places that need to change when
creating a new module.

For example, the following files are involved in defining a new module named
`scoreboard`:

```text
src/
  scoreboard/
    Makefile.am
    scoreboard.c
    scoreboard.h
    README.md
  m65tool/
    Makefile.am
  Makefile.am
tests/
  scoreboard/
    Makefile.am
    test_scoreboard.c
  Makefile.am
configure.ac
```

- `src/scoreboard/` : Source files for the module. At minimum, this is one `.h`
  and one `.c` named after the module that declare and define the module's
  exports. This directory can include inner compilation units (not exported)
  with their own `.c` and `.h` files.
- `src/scoreboard/Makefile.am` : Automake definitions that specify all source
  files, the header files of dependencies, and boilerplate for the mock library
  for this module.
- `src/scoreboard/README.md` : Developer documentation for the scoreboard
  module.
- `src/m65tool/Makefile.am` : Linker rule that lists the new module's library.
- `src/Makefile.am` : Lists every module in `SUBDIRS`.
- `tests/scoreboard/` : At least one file named `test_*.c` containing a test
  suite for the module. The `main()` runner is generated
  at build time from naming conventions in this file to avoid having to
  maintain duplicate lists of tests. This `.c` file `#include`s the header of
  the module under test, the headers of mocked dependencies, and `"unity.h"`
  for test assertions.
- `tests/scoreboard/Makefile.am` : Automake definitions that specify all source
  files for the test, header files, and link dependencies on the module and
  mock libraries. A reference to a "runner" source file triggers the generation
  of the runner code.
- `tests/Makefile.am` : Lists every module in `SUBDIRS`.
- `configure.ac` : Every `Makefile` (without the `.am` extension) _must_ be
  mentioned in `AC_CONFIG_FILES`, including both `src/` and `tests/`.

### Module source files

Here is an example `src/scoreboard/scoreboard.h`:

```c
#ifndef SCOREBOARD_H_
#define SCOREBOARD_H_

/**
 * Scoreboard scores for a two-team game.
 */
typedef struct t_board {
  int home_team_score;
  int away_team_score;
} scoreboard_board;

/**
 * @brief Add two numbers.
 * @param a first addend
 * @param b second addend
 * @returns The sum of a and b
 */
int scoreboard_add(int a, int b);

#endif
```

And here is an example `src/scoreboard/scoreboard.c`:

```c
#include "scoreboard.h"

static const uint8_t BUFSIZE = 256;
static char buf[BUFSIZE];

static int sum(a, b) {
  return a + b;
}

int scoreboard_add(int a, int b) {
  return sum(a, b);
}
```

Modules use a name prefix as a namespace. Module exports start with the name of
the module followed by an underscore, e.g. `scoreboard_add`.

All headers are define-guarded. The public header for the module uses the
symbol `SCOREBOARD_H_`. (The trailing underscore is a GNU convention.)

A module can have additional compilation units (source files) private to the
module.

```text
src/scoreboard/
  ...
  priv.c
  priv.h
```

Inner compilation units that export symbols to other compilation units in the
module _must also_ use a prefix, with a leading underscore to indicate that
the symbol is private to the module, e.g. `_scoreboard_supportfunc`. (If an
inner compilation unit might have a public name that conflicts with another,
you may use the name of the compilation unit in the prefix, such as
`_scoreboard_priv_supportfunc`.) The define guard in the header uses a similar
prefix: `_SCOREBOARD_PRIV_H_`.

File-scope functions and storage must be declared `static`. They do not need a
name prefix.

Anything declared in a header file must have a doc comment. Style notes:

- Opening double-star comment: `/** ... */`
- First sentence is a brief description with the item as an implied noun: "Adds
  two numbers," not "Add two numbers."
- For functions: `@brief desc`, `@param paramName desc`, `@returns desc` Longer
  description can follow the brief description and precede the parameter list.
- For typedefs: one comment above the typedef, one comment above each element.
- For global storage: one comment above. (But prefer non-exported global
  storage and exported accessor functions.)

### Depending on other modules

A module can depend on another module like so:

- Declare the dependency's header file as a source file in `Makefile.am`. (See
  below.)
- `#include "module/module.h"` where appropriate.

The `src/` directory is on the include path, so the `#include` path includes
the module directory name. A module can offer more than one public header file.
This fact would be documented in the module's `README.md` file.

Linkage occurs in the binary, i.e. the module that defines `main()`. The binary
must link the module and all of its dependencies. In practice, the m65tool will
link all modules, and test runners will link only the module under test and the
mock libraries of its dependencies. Module dependencies must be documented
manually in the module's `README.md` file.

### Module tests and mocks

Unit tests use the [Unity Test](http://www.throwtheswitch.org/unity) framework
and the [CMock](http://www.throwtheswitch.org/cmock) mocking framework. These
are added to the project as submodules. C sources of the frameworks are built
into the project, and build rules invoke code generators.

A test file such as `test_scoreboard.c` might look like this:

```c
#include "scoreboard/scoreboard.h"
#include "dependency/mock_dependency.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_Square_WithPositiveInteger_ReturnsCorrectResult(void) {
  dependency_mult_ExpectAndReturn(7, 7, 49);
  int result = examplemod_square(7);
  TEST_ASSERT_EQUAL_INT(49, result);
}
```

- `#include` the header of the module under test.
- `#include` the header for the mock of each dependency.
- `#include "unity.h"` for the test assertions.
- `setUp()` and `tearDown()` are called before and after each test,
  respectively.
- `void test_...(void) { ... }` performs a single test and asserts validation.

The `test_` prefix of the source filename and the `test_` prefix of the test
functions are required. They are used by the code generator to produce a test
runner `main()` routine.

The rest of the test function name should be:

1. The method name
2. The state under test
3. The expected behavior

Each part is described in CamelCase and separated by underscores. As a matter
of style, this is the only code that uses CamelCase names.

After the Makefiles are set up (see below), to run all tests:

```text
make check
```

To run a specific test suite:

```text
make check TESTS=test_scoreboard
```

The runner programs are binary programs and can be used with a debugger. To
build a specific test suite:

```text
make -C tests/examplemod test_examplemod
```

To run its binary:

```text
./tests/examplemod/test_examplemod
```

### Module Makefiles

Each module subdirectory has a `Makefile.am` that builds the module to a
"convenience library." This is a `.a` file marked as `noinst_` so that it does
not get installed on the system, only linked to binaries. In
`src/scoreboard/Makefile.am`:

```makefile
include ../module_common.mk

noinst_LIBRARIES = libscoreboard.a
libscoreboard_a_SOURCES = \
	scoreboard.c \
	scoreboard.h \
	priv1.c \
	priv1.h \
	../dependency/dependency.h

check_LIBRARIES = libscoreboard_mock.a
libscoreboard_mock_a_SOURCES = \
	mock_scoreboard.c \
	mock_scoreboard.h
libscoreboard_mock_a_CPPFLAGS = $(MOCK_CPPFLAGS)
```

The dependency module is only mentioned by its header file. Source files
internal to the dependency module must not be mentioned. Linkage is resolved in
the binary module, such as in `src/m65tool/Makefile.am`:

```makefile
m65tool_LDADD = \
	../scoreboard/libscoreboard.a \
	../dependency/libdependency.a
```

The module's test directory also has a `Makefile.am`:

```makefile
check_PROGRAMS = test_scoreboard
include ../test_common.mk

test_scoreboard_SOURCES = \
	./runners/runner_test_scoreboard.c \
	test_scoreboard.c \
	../../src/scoreboard/scoreboard.h \
	../../src/dependency/mock_dependency.h \
	$(CMOCK_SOURCES)
test_scoreboard_LDADD = \
	../../src/scoreboard/libscoreboard.a \
	../../src/dependency/libdependency_mock.a
test_scoreboard_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/dependency
```

- Use this pretty much verbatim, where `scoreboard` is the module under test
  and `dependency` is an example of a dependency module.
- Unit tests only test the behavior of the module. Calls to other modules
  should be mocked using their `src/*/mock_*.h` header and
  `src/*/lib*_mock.a` library. The test code generator produces these source
  files based on the dependency module's header file.

Add the source and test module subdirectories to `SUBDIRS` in both
`src/Makefile.am` and `tests/Makefile.am`:

```makefile
SUBDIRS = \
  dependency \
  m65tool \
  scoreboard
```

Finally, add the module's source `Makefile` and test `Makefile` (without the
`.am` suffix) to `AC_CONFIG_FILES` in `configure.ac`:

```makefile
AC_CONFIG_FILES(
  [Makefile]
  [src/Makefile]
  [src/scoreboard/Makefile]
  [tests/scoreboard/Makefile]
  ...
)
```

Be sure to re-run `./configure` after making these changes.

## Adding files to the distribution

`EXTRA_DIST` in `Makefile.am` lists all files that aren't build dependencies
that should be added to the final distribution. This should include every file
in `docs/`, for example.

## Producing and testing the distribution

To make the distribution:

```text
make distcheck
```

This produces a file such as `m65tool-0.1.tar.gz`. This is the file someone
will download to build and install the tool from source.

The `distcheck` target creates the distribution, and also runs through an
isolated test build and check. You can test the distribution file manually like
so:

```text
tar xzf m65tool-0.1.tar.gz
cd m65tool-0.1
./configure
make
```

## VSCode

The project repo includes a `.vscode/settings.json` file for [Visual Studio
Code](https://code.visualstudio.com/) users. It sets some formatting variables,
and also sets `"C_Cpp.default.includePath"` so VSCode can find headers.

GNU Autotools generates files inside the `third-party/` submodule directories.
Git is configured to "ignore dirty" changes in these files when reporting repo
status. Unfortunately, VSCode does not yet support this. It will report that
files in `third-party/` have changed in the source and version control panels.
You can ignore these.

VSCode includes a built-in version of `clang-format`, which works with the
provided `.clang-format` settings file. I like to have VSCode aggressively
auto-format code, so there is limited need to run `clang-format`
(`make format`) manually.

The project includes task configurations for debugging unit test suites. With a
`test_modname.c` file open, Run > Start Debugging (F5) and select "Debug test
suite." This generates and builds the test runner, then runs the test in the
debugger. Use VSCode breakpoints and related features.

## Style

m65tool uses standard C17. GNU extensions are disabled to increase the chances
that the code is compatible with other compilers such as Visual Studio 2019.

Code style is based on the [Google C/C++ Style
Guide](https://google.github.io/styleguide/cppguide.html). It is enforced by
`clang-format`. There's a `make format` target that runs it.

- Indents are two spaces. No tabs, unless in `Makefile.am` where tabs are
  significant.

- Ordering of `#include`. (clang-format will re-order correctly.)

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

Other C best practices:

- Prefer `sizeof(var)` to `sizeof(type)`.

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
