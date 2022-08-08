#!/usr/bin/env python3

# Cleans all git-ignored files and empty directories out of the project.
#
# Use --dry-run to print what would be deleted without actually deleting.
#   python3 superclean.py --dry-run
#
# Unlike the Autotools clean targets (clean, distclean, maintainer-clean), this
# script makes an aggressive attempt at reducing the source directory to just
# the files that are—or would be—committed to git. This requires re-running the
# full Autotools set-up to return to a buildable state:
#    autoreconf --install
#    ./configure
#    make distcheck
#
# This shouldn't be necessary *if the Makefiles are written correctly,* but I
# found it too easy for an error in a Makefile to generate a file and not
# remember to clean it up normally, causing builds to fail or succeed when they
# shouldn't. Once Makefiles are stable, Autotools does a good job of detecting
# changes and re-generating files consistently.
#
# https://www.gnu.org/software/automake/manual/html_node/Clean.html

import collections
import os.path
import shutil
import subprocess
import sys


dry_run = False
if len(sys.argv) > 1:
    if sys.argv[1] != '--dry-run':
        print('Usage: python3 superclean.py [--dry-run]')
        sys.exit(1)
    dry_run = True

if not os.path.exists('.git'):
    print('Please run this from the project root directory.')
    sys.exit(1)

if not shutil.which('git'):
    print('Cannot find git. Is it on the command path?')
    sys.exit(1)

result = subprocess.run(
    ['git', 'ls-files', '--others', '--ignored', '--exclude-standard'],
    capture_output=True)

if result.returncode:
    print('git returned an error.')
    sys.exit(1)

files_to_delete = str(result.stdout, encoding='utf-8').split('\n')
for fpath in files_to_delete:
    fpath = fpath.strip()
    if fpath:
        if dry_run:
            print(f'Would delete file {fpath}')
        else:
            print(f'Deleting file {fpath}')
            os.remove(fpath)

fcounts: collections.defaultdict[str, int] = collections.defaultdict(int)
for (dname, dnames, fnames) in os.walk('.'):
    if dname == './.git' or dname.startswith('./.git/'):
        continue
    dparts = dname.split(os.path.sep)
    for i in range(1, len(dparts)+1):
        if i > 1:
            partial = os.path.join(*dparts[:i])
        else:
            partial = dparts[0]
        fcounts[partial] += len(fnames)
dirs_to_delete = [p for p in fcounts if fcounts[p] == 0]
dirs_to_delete.sort(reverse=True)
for dpath in dirs_to_delete:
    if dry_run:
        print(f'Would delete empty directory {dpath}')
    else:
        print(f'Deleting empty directory {dpath}')
        os.rmdir(dpath)

if dry_run:
    print('\n--dry-run specified, so no actual deletes occurred.')
