#!/usr/bin/bash
# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

# Check how every implementation handles input that the main test input can't
# cover.  1e-999, -1e-999, and values just above the limit are left out, because
# each parser is allowed to treat them differently.

declare -r COMMANDS=(
    'awk -f split-seconds.awk'
    './split-seconds-c'
    'python3 split-seconds-decimal.py'
    'pypy3 split-seconds-decimal.py'
    './split-seconds-FMT'
    './split-seconds-FMT_INT'
    './split-seconds-google'
    'java SplitSeconds'
    './split-seconds-oss'
    'perl split-seconds.pl'
    'python3 split-seconds.py'
    'pypy3 split-seconds.py'
    './split-seconds-to_s'
)

# Each entry is an input line, a tab, and the expected output.
declare -r ACCEPTED=(
    $'5\t5.000s'
    $'1e3\t16m 40s'
    $' 5 \t5.000s'
    $'+5\t5.000s'
    $'.5\t0.500s'
    $'5.\t5.000s'
    $'0\t0.000s'
    $'-0\t0.000s'
    $'-0.0\t0.000s'
    $'0.00001\t0.000s'
    $'1020.5\t17m'
    $'59.9996\t1m'
    $'\t'
    $'   \t'
    $'9007199254713599\t104249991373d 23h 59m 59s'
    $'9007199254740992\t104249991374d 7h 36m 32s'
)

declare -r REJECTED=(
    abc 5abc '5 6' 5e 5,5 + - . +-5 ++5 --5 1_000 5d 5f
    0x10 0x1p+53
    inf +inf -inf Infinity nan NaN
    -5 -1e-10 -1e-320 -inf
    1e999 1e30 9007199254740994
)

declare -i FAILURES=0

function fail
{
    printf 'FAIL: %s: %q: %s\n' "$1" "$2" "$3" 1>&2
    FAILURES+=1
}

for COMMAND in "${COMMANDS[@]}"
do
    for ENTRY in "${ACCEPTED[@]}"
    do
        INPUT="${ENTRY%%$'\t'*}"
        EXPECTED="${ENTRY#*$'\t'}"
        # shellcheck disable=SC2086
        ACTUAL="$(printf '%s\n' "$INPUT" | $COMMAND 2>&1)"
        STATUS=$?
        [[ $STATUS -eq 0 ]] || fail "$COMMAND" "$INPUT" "exit status $STATUS"
        [[ "$ACTUAL" == "$EXPECTED" ]] ||
            fail "$COMMAND" "$INPUT" "expected '$EXPECTED', got '$ACTUAL'"
    done

    for INPUT in "${REJECTED[@]}"
    do
        # shellcheck disable=SC2086
        ACTUAL="$(printf '%s\n' "$INPUT" | $COMMAND 2>&1 > /dev/null)"
        STATUS=$?
        [[ $STATUS -eq 1 ]] || fail "$COMMAND" "$INPUT" "exit status $STATUS"
        [[ "$ACTUAL" == Error:* && "$ACTUAL" != *$'\n'* ]] ||
            fail "$COMMAND" "$INPUT" "unexpected message '${ACTUAL:0:60}'"
    done

    # An error stops the run, but earlier lines still print.
    # shellcheck disable=SC2086
    ACTUAL="$(printf '1\nabc\n2\n' | $COMMAND 2> /dev/null)"
    [[ "$ACTUAL" == '1.000s' ]] || fail "$COMMAND" '1,abc,2' "expected '1.000s', got '$ACTUAL'"

    # A read error must not look like success.  The message varies, because gawk and Python can
    # stop with their own before the program's code handles it.
    # shellcheck disable=SC2086
    $COMMAND > /dev/null 2>&1 0> /dev/null && fail "$COMMAND" 'write-only stdin' 'exit status 0'
    # shellcheck disable=SC2086
    $COMMAND > /dev/null 2>&1 < / && fail "$COMMAND" 'directory as stdin' 'exit status 0'
done

if (( FAILURES > 0 ))
then
    printf '%d input validation checks failed\n' "$FAILURES" 1>&2
    exit 1
fi

printf 'Input validation checks passed\n'
