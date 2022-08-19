# Modules

The source code of m65tool is organized into _modules._ Each module has a
directory under `src/`, a directory under `tests/`, a header of exported
functions and globals, and a `module.cfg` file in the source root that declares
the module type (library or program) and module dependencies. The
`scripts/makemake.py` tool generates the GNU Autotools `Makefile.am` for the
project based on this layout.

For example, the following files are involved in defining a new module named
`scoreboard`:

```text
src/
  scoreboard/
    scoreboard.c
    scoreboard.h
    README.md
    module.cfg
  m65tool/
    m65tool.c
    module.cfg
tests/
  scoreboard/
    test_scoreboard.c
```

- `src/scoreboard/` : Source files for the module. At minimum, this is one `.h`
  and one `.c` named after the module that declare and define the module's
  exports. This directory can include inner compilation units (not depended on
  outside of the module) with their own `.c` and `.h` files.
- `src/scoreboard/README.md` : Developer documentation for the scoreboard
  module.
- `tests/scoreboard/` : At least one file named `test_*.c` containing a test
  suite for the module. The `main()` runner is generated
  at build time from naming conventions in this file to avoid having to
  maintain duplicate lists of tests. This `.c` file `#include`s the header of
  the module under test, the headers of mocked dependencies, and `"unity.h"`
  for test assertions.

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

## module.cfg and dependencies

There are two kinds of module: a _library_ module and a _program_ module. The
`module.cfg` file for a library module looks like this:

```ini
[module]
library = scoreboard
```

The `module.cfg` for a program module looks like this:

```ini
[module]
program = m65tool
deps = scoreboard executor reporter
```

The `deps` definition is a space-delimited list of module names on which the
module depends. Both library and program modules can have other modules as
dependencies.

Within the module source code, the module can include a dependency's header
using the path relative to `src/`, so like:

```c
#include "executor/executor.h"
```

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

After running `makemake.py` to generate `Makefile.am`, to run all tests:

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
make tests/runners/test_examplemod
```

To run its binary:

```text
./tests/runners/test_examplemod
```

## Custom Makefile.am rules

A module source directory can have an optional `module.mk` file containing
custom definitions for `Makefile.am`. It is included in the `Makefile.am` just
after the definitions for the module. For example, `module.mk` could extend
these GNU Autotools list variables with customizations not supported directly
by `makemake.py`:

- `lib{modname}_la_LIBADD` for library modules
- `{modname}_LDADD` for program modules
- `tests_runners_{suitename}_SOURCES`
- `tests_runners_{suitename}_LDADD`
- `tests_runners_{suitename}_CPPFLAGS`

The project can have an optional `project.mk` file in the root directory. This
is included at the end of `Makefile.am`, and could be useful for defining
custom rules and extending global list variables.

These files are inserted directly into the `Makefile.am`. They must contain
valid Automake definitions. Note that while Automake can pass through some rule
text directly to the generated Makefile, not all Makefile features are
supported by Automake.
