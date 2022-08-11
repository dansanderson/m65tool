# Developing m65tool

These are instructions for developing m65tool. If you just want to build
m65tool from source for yourself, you can download the source distribution. See
README.md.

See also:

- [Developer cheat sheet](dev_cheat_sheet.md)
- [Modules](dev_modules.md)
- [Style](dev_style.md)

## Prerequisites

Make sure you have the following support tools installed:

- A [gcc](https://gcc.gnu.org/)-compatible C compiler
- [GNU Autotools](https://www.gnu.org/software/automake/manual/html_node/index.html)
- [Ruby](https://www.ruby-lang.org/en/) 2.x or later, for unit test code
  generation

### Prerequisites on Linux

Linux can usually get all dependencies via `apt-get`. Check your OS's software
packaging system for details. On Ubuntu:

```text
sudo apt-get update
sudo apt-get install build-essential autotools-dev autoconf ruby-full git clang-format
```

It is possible to build the Windows binary from Linux, with additional
libraries and alternate configuration options. See "Cross-compiling a Windows
binary from Linux," below.

### Prerequisites on macOS

Mac users are strongly recommended to install [Homebrew](https://brew.sh/) to
manage tool installation. Installing Homebrew also installs the XCode Command
Line Tools, which provides a gcc-compatible C compiler and GNU Automake. Once
installed, run the following command to install the remaining tools:

```text
brew install ruby git clang-format
```

### Prerequisites on Windows

Windows users can [install msys2](https://www.msys2.org/#installation) for the
initial tool set and the `pacman` package manager. Install the remaining
dependencies, including the MinGW toolchain:

```text
pacman -S make mingw-w64-x86_64-toolchain mingw-w64-x86_64-libusb git clang autotools
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

Run `make` to produce `./src/m65tool/m65tool.exe`.

```text
make
```

## Wrangling the intermediate files

You probably noticed that `autoreconf`, `./configure`, `make`, and `make check`
produce dozens of intermediate files strewn about all of the source
directories. It is usually safe to ignore them. Git does: all intermediate
files are mentioned in `.gitignore`, so they don't appear as changed and won't
be committed to the repo.

There are cleaning targets included by Autotools:

- `make clean` deletes build artifacts, just enough to inspire a subsequent
  `make` to re-build most things.

- `make distclean` also deletes some output by `./configure`. You must re-run
  `./configure` before you can `make` again.

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

## Building to a different directory

This behavior of creating build files in the source tree is the default for GNU
Autotools when run from the project root directory. You can generate the build
tree in another directory, like so:

```text
mkdir build
cd build
../configure
make
make check
```

Note that `autoreconf --install` and `../configure` still emit intermediate
build artifacts in the source tree. This only relocates the output of `make`
targets (assuming the `make` rules are written correctly).

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
line. Note that this quiets output for everyone, not just you.

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

## scripts/build.py

I wrote a build invocation script so I wouldn't forget some of these options.
It's purposefully _not_ a Makefile rule because it provides standard options to
`./configure`. Run it from the project root directory:

```text
python3 scripts/build.py
```

Options:

- `--debugbuild` : Produce a debug-enabled binary; default is non-debug optimized
- `--windows` : Build a Windows binary from Linux; default is native OS

This re-runs all build steps, then copies the `m65tool` or `m65tool.exe` binary
to the `./bin/` directory.

## Adding files to the distribution

`EXTRA_DIST` in `Makefile.am` lists all files that aren't build dependencies
that should be added to the final distribution. This should include every file
in `docs/`, for example.

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
