# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

export LC_ALL = C

# https://stackoverflow.com/a/23080721
SHELL = /usr/bin/bash

TMP_DIR = .
INPUT_FILE = $(TMP_DIR)/split-seconds.input.txt
TIME_CMD = /usr/bin/time

# The default collector sizes its heap by timing and total memory, so its peak memory swings
# from run to run.  The serial collector keeps the figure comparable between runs.
JAVA_OPTS = -XX:+UseSerialGC

#CFLAGS += -pg
#CXXFLAGS += -pg
CFLAGS += -O3 -flto=auto -march=native
CXXFLAGS += -O3 -flto=auto -march=native

BINS = \
split-seconds-c \
split-seconds-FMT \
split-seconds-FMT_INT \
split-seconds-google \
split-seconds-oss \
split-seconds-to_s \
SplitSeconds.class

OUTPUT_FILES = \
$(TMP_DIR)/split-seconds-awk.output.txt \
$(TMP_DIR)/split-seconds-c.output.txt \
$(TMP_DIR)/split-seconds-decimal-py.output.txt \
$(TMP_DIR)/split-seconds-decimal-pypy.output.txt \
$(TMP_DIR)/split-seconds-FMT.output.txt \
$(TMP_DIR)/split-seconds-FMT_INT.output.txt \
$(TMP_DIR)/split-seconds-google.output.txt \
$(TMP_DIR)/split-seconds-java.output.txt \
$(TMP_DIR)/split-seconds-oss.output.txt \
$(TMP_DIR)/split-seconds-perl.output.txt \
$(TMP_DIR)/split-seconds-py.output.txt \
$(TMP_DIR)/split-seconds-pypy.output.txt \
$(TMP_DIR)/split-seconds-to_s.output.txt

all: $(BINS) $(INPUT_FILE)

split-seconds-c: split-seconds.c
	$(CC)  $(CPPFLAGS) $(CFLAGS)                 $^ -o $@ -lm

split-seconds-FMT: split-seconds-fmt.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)               $^ -o $@ -lfmt

split-seconds-FMT_INT: split-seconds-fmt.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DUSE_FMT_INT $^ -o $@ -lfmt

split-seconds-google: split-seconds-google.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)               $^ -o $@ -ldouble-conversion

split-seconds-to_s: split-seconds-to_s.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)               $^ -o $@

split-seconds-oss: split-seconds.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)               $^ -o $@

SplitSeconds.class: SplitSeconds.java
	javac                                        $^

$(INPUT_FILE): create-input-file.bash
	bash -x $< $@

$(TMP_DIR)/split-seconds-c.output.txt: split-seconds-c                          $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' ./$<                < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-FMT.output.txt: split-seconds-FMT                      $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' ./$<                < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-FMT_INT.output.txt: split-seconds-FMT_INT              $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' ./$<                < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-google.output.txt: split-seconds-google                $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' ./$<                < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-to_s.output.txt: split-seconds-to_s                    $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' ./$<                < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-oss.output.txt: split-seconds-oss                      $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' ./$<                < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-awk.output.txt: split-seconds.awk                      $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' awk -f $<           < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-java.output.txt: SplitSeconds.class                    $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' java $(JAVA_OPTS) $(basename $<) < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-perl.output.txt: split-seconds.pl                      $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' perl $<             < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-pypy.output.txt: split-seconds.py                      $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' pypy3 $<            < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-decimal-pypy.output.txt: split-seconds-decimal.py      $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' pypy3 $<            < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-py.output.txt: split-seconds.py                        $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' python3 $<          < $(INPUT_FILE) > $@ || $(RM) -- $@

$(TMP_DIR)/split-seconds-decimal-py.output.txt: split-seconds-decimal.py        $(INPUT_FILE)
	$(TIME_CMD) --format '# %E\n# %M KB' python3 $<          < $(INPUT_FILE) > $@ || $(RM) -- $@

# Takes about 1 minute
bench: $(OUTPUT_FILES)
	md5sum $^ | sort -i

check: $(BINS)
	bash check-input-validation.bash

clean-input:
	@$(RM) -v -- $(INPUT_FILE)

clean-output:
	@$(RM) -v -- $(OUTPUT_FILES)

clean: clean-input clean-output
	@$(RM) -v -- $(BINS)

.PHONY: all bench check clean-input clean-output clean

# https://www.gnu.org/software/make/manual/html_node/Parallel-Disable.html
.NOTPARALLEL: bench
