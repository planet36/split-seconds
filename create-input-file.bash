#!/usr/bin/bash
# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

declare -r RESULT_FILE="$1"

if [[ -z "$RESULT_FILE" ]]
then
    printf 'Error: Must give result file to create\n' 1>&2
    exit 1
fi

{
seq 0 0.00001 10
seq 0 0.0001  100
seq 0 0.001   1000
seq 0 0.01    10000
seq 0 0.1     100000
seq 0 1.0     1000000
seq 0 10      10000000
printf '%s\n' 9007199254713599 9007199254740991 9007199254740992
} | sort -g -u -o "$RESULT_FILE"

# This alternative runs each seq in parallel.  Its result is identical, but it
# isn't measurably faster.  Most of the time is spent in sort, which reads its
# inputs one at a time.
# sort -g -u -o "$RESULT_FILE-NEW" \
# <(seq 0 0.00001 10      ) \
# <(seq 0 0.0001  100     ) \
# <(seq 0 0.001   1000    ) \
# <(seq 0 0.01    10000   ) \
# <(seq 0 0.1     100000  ) \
# <(seq 0 1.0     1000000 ) \
# <(seq 0 10      10000000)
