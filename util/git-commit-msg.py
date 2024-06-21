#!/usr/bin/env python3
#
# Copyright (c) 2019 Inria
# All rights reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# If your commit has been canceled because of this file, do not panic: a
# copy of it has been stored in ./.git/COMMIT_EDITMSG

import os
import re
import sys

from style.repo import GitRepo


def _printErrorQuit(error_message):
    """
    Print an error message, followed my a help message and inform failure.

    @param error_message A message describing the error that caused the
        failure.
    """
    print(error_message)

    print(
        "The commit has been cancelled, but a copy of it can be found in "
        + sys.argv[1]
        + " : "
    )

    print(
        """
--------------------------------------------------------------------------
    """
    )
    print(open(sys.argv[1]).read())
    print(
        """
--------------------------------------------------------------------------
    """
    )

    print(
        """

This header (title) line must be provided as a summary of the commit's change.
It cannot not exceed 65 characters in length. The header can then be followed
by a more detaoled message seperated from the header by an empty line with no
trailing whitespace. A detailed message is highly recommended but not
mandatory. Likewise we recommend that the description not exceed 72 characters
in width though this is not enforced.


e.g.:
    Refactor branch predictors

    Refactor branch predictor code to improve its readability, moving functions
    X and Y to the base class...

e.g.:
    Improve packet class readability

    The packet class...
"""
    )
    sys.exit(1)


# Go to git directory
os.chdir(GitRepo().repo_base())

# Get the commit message
commit_message = open(sys.argv[1]).read()

# The first line of a commit must contain at least one valid gem5 tag, and
# a commit title
commit_message_lines = commit_message.splitlines()
commit_header = commit_message_lines[0]

# Make sure commit title does not exceed threshold. This line is limited to
# a smaller number because version control systems may add a prefix, causing
# line-wrapping for longer lines
max_header_size = 65
if len(commit_header) > max_header_size:
    _printErrorQuit(
        "The commit header (title) is too long ("
        + str(len(commit_header))
        + " > "
        + str(max_header_size)
        + ")"
    )

# Then there must be at least one empty line between the commit header and
# the commit description
if commit_message_lines[1] != "":
    _printErrorQuit(
        "Please add an empty line between the commit title and "
        "its description"
    )

# Encourage providing descriptions
if re.search(
    "^(Signed-off-by|Change-Id|Reviewed-by):", commit_message_lines[2]
):
    print("Warning: Commit does not have a description")

sys.exit(0)
