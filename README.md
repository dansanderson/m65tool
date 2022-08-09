# m65tool: a command-line multitool for MEGA65 owners and developers

m65tool is a companion tool for the [MEGA65](https://mega65.org/), a modern
recreation of the unreleased [Commodore
65](https://en.wikipedia.org/wiki/Commodore_65) microcomputer (a sequel to
the Commodore 64).

**August 2022:** Development of m65tool is in progress. It is intended to be a
new frontend for the `m65`, `mega65_ftp`, and `m65dbg` tools, with similar
features and an enhanced user interface. Planned features include:

- Use as a unified command-style interface available as an interactive terminal
  prompt, a non-interactive shell command for automation, and a local server
  for UI frontends like M65Connect.
- Transfer files to and from the SD card.
- Transmit PRG files, bitstreams, ROM files, and raw data to MEGA65 memory.
- Capture screenshots as PNG files.
- Reset the MEGA65 in C65 or C64 mode, PAL or NTSC.
- Send text as if typed on the keyboard.
- Interact remotely with an ANSI representation of the text screen and remote
  keyboard typing.
- Debug programs remotely with an enhanced machine language monitor,
  breakpoints, function stepping, assembly source symbols, arithmetic
  expressions, real-time assembly to memory, disassembly display with assembly
  source mapping, memory manipulation, and register and interrupt inspection.
- All debugging features run in tandem with your program without cluttering the
  MEGA65 display.
- Configure operation of the tool across sessions and projects with
  configuration files.
- Enhanced serial connection management, including automatic port detection and
  as-needed connections.
- Access an online help guide for every feature and assembly instruction.
- Run a local TCP/IP server for other apps and tools to issue commands. Bind
  the server to a LAN/WAN IP address for access over the network.
- Use machine-readable JSON commands and responses via stdin/stdout or local
  server interfaces.
- Clean extensible architecture for adding new commands and unit tests.
- Build infrastructure based on GNU Autotools capable of producing Windows,
  Mac, Linux, and source distributions.

And if we can manage all of that, why not:

- Browser UI for local server.
- TCP/IP network security, user authentication and per-command authorization.

m65tool connects to a MEGA65 using [a JTAG or serial USB
connection](https://dansanderson.com/mega65/welcome/using-jtag.html). It can
also perform some functions connected to [the Xemu MEGA65
emulator](https://github.lgb.hu/xemu/) behaving as if it were a MEGA65.

## Who is doing this?

Oliver Graf (lydon) maintains the
[mega65-tools](https://github.com/MEGA65/mega65-tools) repo. Gurce Isikyildiz
(Gurce) wrote and maintains [m65dbg](https://github.com/MEGA65/m65dbg). Dan
Sanderson (dddaaannn) is driving the frontend rewrite.

## Why is it a separate tool?

The new frontend will be a proof of concept at first. We don't want to make
backwards-incompatible modifications to the existing tool interfaces, on which
other tools and automations are already built. While it is possible that
m65tool may replace tools like `m65`, `mega65_ftp`, and `m65dbg`, it is far
from necessary.

It is a goal for all of these tools to share common feature code. Code sharing
interfaces will be developed as part of the m65tool project. m65tool is a
single statically-linked binary and does not require the other tools be
installed.

## How do I install it?

During early development, m65tool is only available via [the m65tool Github
repo](https://github.com/dansanderson/m65tool). When the tool is ready to use,
it will be available via application and source distributions for all
platforms.

Windows users will be supported by a [MinGW](https://www.mingw-w64.org/)-based
build workflow, similar to mega65-tools. You're welcome to try getting the
Linux instructions to work via Cygwin, but this isn't set up or tested for
Windows yet.

Linux and Mac users, make sure you have the following support tools installed:

- A [gcc](https://gcc.gnu.org/)-compatible C compiler
- [GNU Automake](https://www.gnu.org/software/automake/manual/html_node/index.html)
- [Ruby](https://www.ruby-lang.org/en/) 2.x or later, for unit test code
  generation

Linux can usually get these via `apt-get`. Check your OS's software packaging
system for details. On Ubuntu:

```text
sudo apt-get update
sudo apt-get install build-essential autoconf ruby-full git
```

Mac users are strongly recommended to install [Homebrew](https://brew.sh/) to
manage tool installation. Installing Homebrew also installs the XCode Command
Line Tools, which provides a gcc-compatible C compiler and GNU Automake. Once
installed, run the following command to install the remaining tools:

```text
brew install ruby git
```

### Building m65tool from the Github repo

Clone the `m65tool` Github repo, with submodules (recursively):

```text
git clone https://github.com/dansanderson/m65tool --recurse-submodules
cd m65tool
```

If you cloned the repo some other way, you can get the submodules with:

```text
git submodule update --init --recursive
```

Run Automake's `autoreconf` to generate the initial set of build files,
run the `./configure` script that it produces, then run `make`:

```text
autoreconf --install
./configure
make
```

Changes to `configure.ac` or `Makefile.am` will regenerate the appropriate
files during the next `make`. If for some reason this doesn't work, you can
re-run `autoreconf` as needed.

To run unit tests:

```text
make check
```

The `m65tool` binary is built to `src/m65tool/m65tool`. If you really want to,
you can `make install` to copy this tool to your system's install location. To
change the install location, provide a `--prefix=/path/to/root` argument to
`./configure`. I don't recommend this until the tool is closer to completed.

## Can I contribute?

Thanks for your interest! We're not ready for code contributions just yet. We
will solicit testing and coding help in the [MEGA65 Discord #tools
channel](https://discord.gg/5DNvESf) when the time comes.

If you want to know more about the project, you can [email
Dan](mailto:contact@dansanderson.com) or DM dddaaannn#7325 on the Discord.
