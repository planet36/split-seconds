# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

The same program written several ways, to compare speed, memory use, and output. Each implementation reads one number of seconds per line from stdin and prints it as `1d 1h 1m 2s`. There is no shared code between implementations, so a behavior change has to be made in every source file.

## Comment style

Follow @COMMENT-STYLE.md when writing code comments, commit messages, and documentation.

## Commands

```bash
make                # build all binaries and generate the input file
make bench          # time every implementation, then print md5sums of the outputs (about 1 minute, writes about 850 MB)
make check          # check how every implementation handles valid and invalid input lines, and fail on a mismatch
make clean-output   # delete outputs, required to re-run implementations whose sources are unchanged
make clean          # delete binaries, input, and outputs
make bench TMP_DIR=/some/dir   # put the input and output files elsewhere
```

Run one implementation (there is no test framework, so this is the closest thing to a single test):

```bash
printf '90061.5\n' | ./split-seconds-c                 # try a value directly
rm -f split-seconds-py.output.txt && make split-seconds-py.output.txt   # full input, with timing
diff split-seconds-c.output.txt split-seconds-py.output.txt             # find lines that differ
paste split-seconds.input.txt split-seconds-c.output.txt split-seconds-py.output.txt | awk -F'\t' '$2 != $3'   # show the inputs that differ
```

Output targets are named `split-seconds-<impl>.output.txt`, where `<impl>` is one of `awk`, `c`, `decimal-py`, `decimal-pypy`, `FMT`, `FMT_INT`, `google`, `java`, `oss`, `perl`, `py`, `pypy`, `to_s`.

Don't use `make -B` to re-run one output: it also regenerates the input file.

## Build environment

The build depends on things outside the repo:

- **Headers:** `getline.hpp`, `f_to_s.hpp`, and `i_to_s.hpp` live in `~/.local/include`. They are found only through `-iquote` in the `CPPFLAGS` environment variable.
- **Language standard:** the Makefile doesn't set one. `CFLAGS` and `CXXFLAGS` in the environment supply `-std=gnu2y` and `-std=gnu++26`, and the Makefile appends to them. The sources use C23 `constexpr` and C++23 `std::println`.
- **Libraries and tools:** `libfmt`, `double-conversion`, `pypy3`, `/usr/bin/time`, and a JDK with `java.lang.IO` (25+).

The `gccc`, `gpp`, and `d` commands in source header comments aren't defined in this repo. Use the Makefile.

## How the tests work

`make bench` never fails on a mismatch.  It prints md5sums, and you read the groups yourself. The expected result, recorded in the checked-in `output.txt` log along with past timings, is two groups:

| Group | Implementations | Why it differs |
|---|---|---|
| Baseline | `awk`, `c`, `FMT`, `FMT_INT`, `google`, `java`, `oss`, `perl`, `py`, `pypy`, `to_s` | Binary doubles, printed with round-half-even |
| Decimal | `decimal-py`, `decimal-pypy` | Parses the input text exactly, so ties land differently (`0.00050` gives `0.000s`, not `0.001s`) |

Each output file holds only the results, one line per input line, so line numbers match the input file.  Timing goes to stderr, not the output files, and it covers a single run that includes writing the output file.  `.NOTPARALLEL: bench` keeps runs serial so timings stay comparable.

The input comes from `create-input-file.bash`: about 6.4 million sorted, unique, non-negative values up to 10,000,000, plus a few values near the limit of 2^53.

`make check` runs `check-input-validation.bash`, which covers what that input can't: lines that must be rejected, blank lines, and accepted spellings of a number.  Unlike `make bench`, it fails on a mismatch.

## Input rules

All implementations share these rules:

- **A valid line** is a decimal number with an optional exponent, from 0 to 2^53 inclusive.  Whitespace around it is allowed, and a blank line prints an empty line.
- **Anything else stops the run** with an `Error:` message and exit status 1.  That covers non-numeric text, hex, NaN, infinity, negative values, and values above the limit.
- **A read error stops the run** with `Error: could not read stdin: <reason>` and exit status 1.  `gawk` and Python can stop first with their own message, so `make check` only requires a nonzero exit.
- **Validation uses each language's own parser**, plus a small check for what that parser accepts beyond these rules.  Avoid a regex on the main path: it makes that implementation validate each line twice, which skews the comparison.  It cost `perl` about 2.5 seconds and `awk` about 12 seconds on the full input.
- **Each parser's result is used as it comes**, so a few extreme values differ.  `1e-999` and `-1e-999` are out of range for `from_chars`.  `decimal` keeps `-1e-999` as a negative value.  `decimal` and `perl` read values just above the limit exactly and reject them, while the others round them down to the limit.

## Output rules

All implementations share this logic, so keep it the same when changing any of them:

- **Precision** comes from the total value before splitting: 3 decimals below 10, 2 below 100, 1 below 1000, otherwise 0. This means `90061.5` prints `1d 1h 1m 2s`.
- **Zero units** are omitted, and seconds that round to zero count as zero, so `1020.5` prints `17m`.  When nothing else would print, `print_zero_seconds` shows zero seconds, so `0` and `0.00001` print `0.000s`.  Only a blank input line prints an empty line.
- **Rounding** is left to the formatter.  Seconds that round up to 60 carry into the larger units, so `59.9996` prints `1m` and `86399.5` prints `1d`.  Each implementation checks the formatted text for this, which keeps the check in step with its formatter.
- **Java** uses `BigDecimal` with `HALF_EVEN` to match `printf`. `String.format` doesn't match, because it rounds the shortest decimal form half-up instead of the exact stored value, so `0.0045` (stored as `0.00449999...`) gives `0.005s`.
- **Google** needs a hand-written check before `ToFixed`, which rounds exact ties up (`0.0625` would give `0.063s`, not `0.062s`).

## Adding an implementation

Add it to `OUTPUT_FILES` and write its output rule in the `Makefile`, and add it to `COMMANDS` in `check-input-validation.bash`.  Compiled implementations also go in `BINS`, and their build outputs need adding to `.gitignore`.
