# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

The same program written several ways, to compare speed, memory use, and output. Each implementation reads one number of seconds per line from stdin and prints it as `1d 1h 1m 2s`. There is no shared code between implementations, so a behavior change has to be made in every source file.

## Comment style

Follow @COMMENT-STYLE.md when writing code comments, commit messages, and documentation.

## Commands

```bash
make                # build all binaries and generate the input file
make test           # run every implementation, then print md5sums of the outputs (about 1 minute, writes about 1.5 GB)
make clean-output   # delete outputs, required to re-run implementations whose sources are unchanged
make clean          # delete binaries, input, and outputs
make test TMP_DIR=/some/dir   # put the input and output files elsewhere
```

Run one implementation (there is no test framework, so this is the closest thing to a single test):

```bash
printf '90061.5\n' | ./split-seconds-c                 # try a value directly
rm -f split-seconds-py.output.txt && make split-seconds-py.output.txt   # full input, with timing
diff split-seconds-c.output.txt split-seconds-py.output.txt             # find lines that differ
```

Output targets are named `split-seconds-<impl>.output.txt`, where `<impl>` is one of `awk`, `c`, `decimal-py`, `decimal-pypy`, `FMT`, `FMT_INT`, `google`, `java`, `oss`, `perl`, `py`, `pypy`, `to_s`.

Don't use `make -B` to re-run one output: it also regenerates the input file.

## Build environment

The build depends on things outside the repo:

- **Headers:** `getline.hpp`, `f_to_s.hpp`, and `i_to_s.hpp` live in `~/.local/include`. They are found only through `-iquote` in the `CPPFLAGS` environment variable.
- **Language standard:** the Makefile doesn't set one. `CFLAGS` and `CXXFLAGS` in the environment supply `-std=gnu2y` and `-std=gnu++26`, and the Makefile appends to them. The sources use C23 `constexpr` and C++23 `std::string::contains`.
- **Libraries and tools:** `libfmt`, `double-conversion`, `pypy3`, `/usr/bin/time`, and a JDK with `java.lang.IO` (25+).

The `gccc`, `gpp`, and `d` commands in source header comments aren't defined in this repo. Use the Makefile.

## How the test works

`make test` never fails on a mismatch. It prints md5sums, and you read the groups yourself. The expected result, recorded in the checked-in `output.txt` log along with past timings, is two groups:

| Group | Implementations | Why it differs |
|---|---|---|
| Baseline | `awk`, `c`, `FMT`, `FMT_INT`, `google`, `java`, `oss`, `perl`, `py`, `pypy`, `to_s` | Binary doubles, printed with round-half-even |
| Decimal | `decimal-py`, `decimal-pypy` | Parses the input text exactly, so ties land differently (`0.00050` gives `0.000s`, not `0.001s`) |

Each output file is the input pasted beside the result (`input<TAB>result`), so a diff shows the value that caused it. Timing goes to stderr, not the output files. `.NOTPARALLEL: test` keeps runs serial so timings stay comparable.

The input comes from `create-input-file.bash`: 6.4 million sorted, unique, non-negative values up to 10,000,000. Anything outside that range is untested, and implementations do disagree there:

- `perl` doesn't take the absolute value, so `-5` prints `-5.000s`.
- `awk` and `perl` have no `max_seconds` clamp.
- Very large values such as `1e30` give a different result in each language, and `c` truncates them to fit its fixed-size buffer.

## Output rules

All implementations share this logic, so keep it the same when changing any of them:

- **Precision** comes from the total value before splitting: 3 decimals below 10, 2 below 100, 1 below 1000, otherwise 0. This means `90061.5` prints `1d 1h 1m 2s`.
- **Zero units** are omitted. A total of exactly zero prints an empty line, because `print_zero_seconds` is false. A blank input line also prints an empty line.
- **Rounding** is left to the formatter and doesn't carry into minutes, so `59.9996` prints `60.00s`.
- **Java** uses `BigDecimal` with `HALF_EVEN` to match `printf`. `String.format` doesn't match, because it rounds the shortest decimal form half-up instead of the exact stored value, so `0.0045` (stored as `0.00449999...`) gives `0.005s`.
- **Google** needs a hand-written check before `ToFixed`, which rounds exact ties up (`0.0625` would give `0.063s`, not `0.062s`).

## Adding an implementation

Add it to `OUTPUT_FILES` and write its output rule in the `Makefile`. Compiled implementations also go in `BINS`, and their build outputs need adding to `.gitignore`.
