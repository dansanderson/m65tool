# Developing m65tool

These are instructions for developing m65tool. If you just want to build
m65tool from source for yourself, you can download the source distribution, and
build it with any POSIX-compatible environment, including Linux, macOS, and
Windows with [MinGW](https://www.mingw-w64.org/). See [README.md](../README.md).

See also:

- [Developer cheat sheet](dev_cheat_sheet.md)
- [Modules](dev_modules.md)
- [Style](dev_style.md)

## Introduction

m65tool is written in C, with development helper scripts in Python and Ruby.
Build logic is managed with GNU Autotools and a simple module management
system. Autotools can generate a source distribution that can be built in any
POSIX-compliant environment without the need for Python or Ruby.

The module management system is a single script, `scripts/makemake.py`. This
tool expects the C code to be organized into modules, as documented by
[dansanderson/c-autotools-template](https://github.com/dansanderson/c-autotools-template).
Each module source directory contains a `module.cfg` file that declares the
module type (library or program) and the module's dependencies. `makemake.py`
generates rules for unit tests and mock dependencies.

## Prerequisites

Make sure you have the following support tools installed:

- A [gcc](https://gcc.gnu.org/)-compatible C compiler
- [GNU Autotools](https://www.gnu.org/software/automake/manual/html_node/index.html)
- [Ruby](https://www.ruby-lang.org/en/) 2.x or later, for unit test code
  generation
- [Python](https://www.python.org/) 3.x for the module management tools
- [lcov](http://ltp.sourceforge.net/coverage/lcov.php) for code coverage

On Linux, you can install these prerequisites with your system's package
manager. For example, on Ubuntu:

```text
sudo apt-get update
sudo apt-get install build-essential autotools-dev autoconf ruby-full git clang-format python3.10 lcov
```

On macOS, install [Homebrew](https://brew.sh/). Simply installing Homebrew also
installs the XCode Command Line Tools, including a gcc-compatible C compiler
and GNU Autotools. You can install additional tools like so:

```text
brew install ruby python git clang-format lcov
```

On Windows, install [MinGW MSYS2](https://www.msys2.org/#installation). The
instructions describe how to open an MSYS terminal and run the `pacman` package
manager. You can use `pacman` to install the MinGW toolchain and other tools:

```text
pacman -S base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-libusb clang autotools git lcov
```

**Note:** Take care to build in a MinGW shell, and not an "MSYS" shell. From the MSYS
shell, builds will require the MSYS DLL to run. A build in the MinGW shell
produces a standalone `.exe` program. Because I always want a MinGW standalone
binary, this template's `configure.ac` will abort if built under MSYS.

The MSYS2 user home directory is under `C:\msys64\home\{user}` (where `{user}`
is your username). If you clone the repo into an `m65tool` subdirectory, the
final `.exe` will be:

```text
C:\msys64\home\{user}\m65tool\m65tool.exe
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

Use `make` to build the tool. The `m65tool` program (or `m65tool.exe` on
Windows) is created in the project root directory.

```text
make
```

Use `make check` to build and run all unit tests and report a failure summary.
This builds a standalone binary for each module, such as `./tests/`.

```text
make check
```

Use `make distcheck` to run all tests and produce the source distribution.

```text
make distcheck
```

## Cross-compiling a Windows binary from Linux

To cross-compile the Windows version from Linux, install additional libraries
and tools:

```text
sudo apt-get install binutils-mingw-w64 mingw-w64-common gcc-mingw-w64 libz-mingw-w64-dev
```

> TODO: mega65-tools is using custom versions of libpng and libusb from Gurce's
> own server. We may need to borrow [those installation
> instructions](https://github.com/MEGA65/mega65-tools/blob/9901cc807e0b92aef1936b664bfa3af6a997cff9/README.md#building-under-linux)
> when we get to that point.

Tell `./configure` the builder OS is Linux and the host (target) OS is Windows:

```text
./configure --build=x86_64-pc-linux-gnu --host=x86_64-w64-mingw32
```

Run `make` to produce `m65tool.exe`.

```text
make
```

## Updating Makefile.am with makemake.py

This project uses a tool called `makemake.py` to generate the `Makefile.am`
file that is processed by `./configure`. It is customary to commit this file to
the source repo.

To update `Makefile.am`:

```text
python3 scripts/makemake.py
```

This tool calculates `Makefile.am` from the module layout in `src/` and
`tests/`, from the `module.cfg` file (required) and `module.mk` (optional) in
each module source directory, and from an optional `project.mk` file in the
project root directory.

You must re-run `makemake.py` whenever a source file is created or deleted, and
whenever a `module.cfg`, `module.mk`, or `project.mk` file is changed. You
typically also want to run `./configure` immediately afterward.

You usually don't need to re-run this tool under other circumstances, but it
doesn't hurt to do so.

## Creating new modules

The files required for a new module are intentionally minimal. Nevertheless,
there's a tool to make it even easier to generate the starter files for a new
module: `scripts/newmod.py`

To generate a new internal library module:

```text
python3 scripts/newmod.py {modname}
```

To generate a new program module:

```text
python3 scripts/newmod.py --program {modname}
```

## Wrangling the intermediate files

`autoreconf`, `./configure`, `make`, and `make check` produce dozens of
intermediate files strewn about all of the source directories. It is usually
safe to ignore them. Git does: all intermediate files are mentioned in
`.gitignore`, so they don't appear as changed and won't be committed to the
repo.

There are cleaning targets included by Autotools:

- `make clean` deletes build artifacts, just enough to inspire a subsequent
  `make` to re-build most things.

- `make distclean` also deletes some output by `./configure`. You must re-run
  `./configure` before you can `make` again.

I found it was important to fully restore the project directory to its Git repo
state to reset artifacts from erroneous Makefile rules. To make this easier,
the tool `scripts/superclean.py` deletes all files explicitly ignored by Git,
and deletes empty directories. You must re-run
`autoreconf --install && ./configure && make` to build again.

To see what files will be deleted without actually deleting:

```text
python3 superclean.py --dry-run
```

To delete all temporary files:

```text
python3 superclean.py
```

## Quieter builds

The build produces a lot of console output. This is the default for GNU
Autotools, and I have left it this way for now for troubleshooting.

To tell `make` to produce less output, such as to make compiler warnings and
errors easier to see, give it the `-s` option:

```text
make -s
make -s check
```

To make this the default for `make`, give `--enable-silent-rules` to
`./configure`:

```text
./configure --enable-silent-rules
make
```

To further make `--enable-silent-rules` the default in `configure.ac`, uncomment this
line. (Don't commit this change: we want full messages as the default.)

```text
AM_SILENT_RULES([yes])
```

## Debug vs. optimized builds

The default build rules for `./configure` set `CFLAGS` such that debugging
symbols are included (`-g`) and the binary is optimized at level 2 (`-O2`).

You can pass new CFLAGS to `./configure` to change this behavior. To disable
optimization so that debuggers have accurate source data, and also set a
`DEBUG` preprocessor variable for conditional compilation:

```text
./configure CPPFLAGS=-DDEBUG CFLAGS="-ggdb -O0"
```

To disable debugging symbols, enable more optimizations, and also set a
`NDEBUG` preprocessor variable for conditional compilation:

```text
./configure CPPFLAGS=-DNDEBUG CFLAGS="-g0 -O3"
```

[This SO answer](https://stackoverflow.com/a/4680578/453278) recommends against
adding these definitions to Makefiles.

## Easier building with scripts/build.py

I wrote a build invocation script so I wouldn't forget some of these options.
It's purposefully _not_ a Makefile rule because it provides standard options to
`./configure`. Run it from the project root directory:

```text
python3 scripts/build.py
```

Options:

- `--debugbuild` : Produce a debug-enabled binary; default is non-debug optimized
- `--windows` : Build a Windows binary from Linux; default is native OS
- `--verbose` : Un-silences make messages; default is to use quieter builds

This re-runs all build steps, then creates a symbolic link to the `m65tool` or
`m65tool.exe` binary in the `./bin/` directory.

You can run `make` after this to re-build with the same options. Re-run
`build.py` to re-run `./configure` with new options.

## Producing the source distribution

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
