# C style and development tools

This document describes a few coding style decisions made for this project,
along with a bunch of notes that C programmers already know but I found handy
to write down.

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

The Makefile Tools extension tries to do a `--dry-run` build to generate a
build log it can read for more information. This has some challenges with
generated source files. There are [some
troubleshooting
tips](https://github.com/microsoft/vscode-makefile-tools/blob/main/docs/troubleshooting.md),
though I don't think I've implemented them fully. If the dry run seems to not
finish, I just cancel, do a full build, and then Reload Window.

## Valgrind

[Valgrind](https://valgrind.org/) is the de facto standard tool for analyzing
programs for performance, memory, and threading bugs.

As of August 2022 and for the foreseeable future, Valgrind does not run on
modern versions of macOS. It's best to run it on Linux or Windows.

To install on Linux:

```text
sudo apt-get valgrind
```

Make sure to build m65tool with all debugging symbols on and optimizations off:

```text
python3 scripts/build.py --debugbuild
```

Use the `valgrind` command with Valgrind arguments, followed by the m65tool
binary and any arguments and streamed input intended for m65tool:

```text
valgrind --leak-check=yes ./bin/m65tool help
```

See [the Valgrind quick
start](https://valgrind.org/docs/manual/quick-start.html) for discussion of the
most common types of memory bug.
