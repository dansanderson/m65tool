# Modules

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

## Module source files

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

## Depending on other modules

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

## Module tests and mocks

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

## Module Makefiles

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

CLEANFILES = mock_scoreboard.c mock_scoreboard.h
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