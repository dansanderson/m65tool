#!/usr/bin/env python3

# Builds m65tool, with help.

import argparse
import os
import os.path
import platform
import shutil
import subprocess
import sys


def error(msg):
    """Prints an error message then exits the program.

    Args:
        msg: The message to print.
    """
    sys.stderr.write(msg + '\n')
    sys.exit(1)


def run(args, verbose=False):
    if verbose:
        print('\n### ' + ' '.join(args))
    result = subprocess.run(args, shell=True)
    return result.returncode


def main(args):
    parser = argparse.ArgumentParser(
        description='Builds m65tool, with help.',
        epilog="""This is a convenience tool for maintainers. It does not
               offer every possible build configuration, nor does it
               necessarily produce final distributions. See docs/developing.md.
               """)
    parser.add_argument(
        '-v', '--verbose', action='store_true',
        help='Enable verbose make messages')
    parser.add_argument(
        '--windows', action='store_true',
        help='Build m65tool.exe for Windows (on Windows or Linux)')
    parser.add_argument(
        '--bindir', default='bin',
        help='Where to put built binaries')
    parser.add_argument(
        '--debugbuild', action='store_true',
        help='Enable debugging symbols, disable optimizations')
    args = parser.parse_args(args)

    if not os.path.exists('.git'):
        error('Please run this from the project root directory.')
    if not shutil.which('autoreconf'):
        error('Cannot find autoreconf. Is GNU Autotools installed?')
    if not shutil.which('make'):
        error('Cannot find make. Are build essentials installed?')

    if not os.path.exists('configure'):
        if run(['autoreconf', '--install'], verbose=args.verbose):
            error('\n*** autoreconf failed, aborting.\n')

    conf_quiet = ['--enable-silent-rules']
    if args.verbose:
        conf_quiet = []

    conf_debug = ['CPPFLAGS=-DNDEBUG', 'CFLAGS="-g0 -O3"']
    if args.debugbuild:
        conf_debug = ['CPPFLAGS=-DDEBUG', 'CFLAGS="-ggbd -O0"']

    conf_crosswindows = []
    if args.windows:
        if platform.system() == 'Windows':
            pass
        elif platform.system() == 'Linux':
            if not shutil.which('x86_64-w64-mingw32-gcc'):
                error('Cannot find x86_64-w64-mingw32-gcc. '
                      'See docs/developing.md for how to install the tools.')
            conf_crosswindows = [
                '--build=x86_64-pc-linux-gnu',
                '--host=x86_64-w64-mingw32']
        else:
            error('Cannot build the Windows version (--windows) from macOS')

    conf_cmd = ['./configure'] + conf_quiet + conf_debug + conf_crosswindows
    if run(conf_cmd, verbose=args.verbose):
        error('\n*** ./configure failed, aborting.\n')

    if run(['make'], verbose=args.verbose):
        error('\n*** make failed, aborting.\n')

    is_windows = (platform.system() == 'Windows') or args.windows
    binname = 'm65tool.exe' if is_windows else 'm65tool'
    buildpath = os.path.join('src/m65tool', binname)
    if not os.path.exists(buildpath):
        error('Could not find binary at expected build path: ' + buildpath)

    os.makedirs(args.bindir, exist_ok=True)
    binpath = os.path.join(args.bindir, binname)
    shutil.copy(buildpath, binpath)
    print('\n' + binpath + ' : build successful')


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
