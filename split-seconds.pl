# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

use strict;
use warnings;
use IO::Handle;
use Scalar::Util qw(looks_like_number);

use constant SECONDS_PER_DAY => 24*60*60;
use constant SECONDS_PER_HOUR => 60*60;
use constant SECONDS_PER_MINUTE => 60;
# This is an integer because Perl compares 2**53 as a double, which lets 2**53 + 1 through.
use constant MAX_SECONDS => 9007199254740992;

# This is the maximum number of digits to print after the decimal point of the seconds value.
#use constant SECONDS_NUM_DIGITS => 2;
#use constant SECONDS_NUM_DIGITS_MULTIPLIER => 10**SECONDS_NUM_DIGITS;
# Print the result if the rounded elapsed time is zero seconds.
use constant PRINT_ZERO_SECONDS => 1;

sub split_seconds {

    my $seconds = $_[0];
    my $result = "";

    # Without this, -0 would print as -0.000s.
    if ($seconds == 0) {
        $seconds = 0;
    }

    my $precision = 0;

    if ($seconds < 1000) {
        ++$precision;
        if ($seconds < 100) {
            ++$precision;
            if ($seconds < 10) {
                ++$precision;
            }
        }
    }

    my $days = 0;
    my $hours = 0;
    my $minutes = 0;

    if ($seconds >= SECONDS_PER_DAY) {
        $days = int($seconds / SECONDS_PER_DAY);
        #$seconds = $seconds % SECONDS_PER_DAY;
        $seconds -= $days * SECONDS_PER_DAY;
    }

    if ($seconds >= SECONDS_PER_HOUR) {
        $hours = int($seconds / SECONDS_PER_HOUR);
        #$seconds = $seconds % SECONDS_PER_HOUR;
        $seconds -= $hours * SECONDS_PER_HOUR;
    }

    if ($seconds >= SECONDS_PER_MINUTE) {
        $minutes = int($seconds / SECONDS_PER_MINUTE);
        #$seconds = $seconds % SECONDS_PER_MINUTE;
        $seconds -= $minutes * SECONDS_PER_MINUTE;
    }

    # Round seconds to SECONDS_NUM_DIGITS digits after the decimal point.
    #$seconds = int($seconds * SECONDS_NUM_DIGITS_MULTIPLIER + 0.5) / SECONDS_NUM_DIGITS_MULTIPLIER;

    my $seconds_str = sprintf("%.*f", $precision, $seconds);

    # Rounding can reach 60 seconds, so carry it into the larger units.
    if (substr($seconds_str, 0, 2) eq "60") {
        $seconds = 0;
        if (++$minutes == SECONDS_PER_MINUTE) {
            $minutes = 0;
            if (++$hours == 24) {
                $hours = 0;
                ++$days;
            }
        }
    }

    # Seconds that round to zero count as zero, so they don't print beside larger units.
    if ($seconds_str == 0) {
        $seconds = 0;
    }

    if ($days != 0) {
        if ($result ne "") {
            $result .= " ";
        }

        $result .= $days . "d";
    }

    if ($hours != 0) {
        if ($result ne "") {
            $result .= " ";
        }

        $result .= $hours . "h";
    }

    if ($minutes != 0) {
        if ($result ne "") {
            $result .= " ";
        }

        $result .= $minutes . "m";
    }

    if ($seconds != 0 || (PRINT_ZERO_SECONDS && ($result eq ""))) {
        if ($result ne "") {
            $result .= " ";
        }

        #$result .= sprintf("%.*g", $precision, $seconds) . "s";
        $result .= $seconds_str . "s";
    }

    return $result;
}

while (my $line = <STDIN>) {

    chomp($line);

    if ($line eq "") {
        print "\n";
        next;
    }

    # looks_like_number also accepts "0 but true", which is the only such text with a t.
    if (!looks_like_number($line) || index($line, "t") >= 0) {
        if ($line =~ /^\s*$/) {
            print "\n";
            next;
        }

        print STDERR "Error: not a number: $line\n";
        exit 1;
    }

    my $seconds = 0;
    $seconds = $line + 0;

    # This also rejects NaN because every comparison with it is false.
    if (!($seconds >= 0 && $seconds <= MAX_SECONDS)) {
        print STDERR "Error: out of range: $line\n";
        exit 1;
    }

    my $result = "";
    #printf("# seconds=%.10f\n", $seconds); #####
    $result = split_seconds($seconds);

    #if ($result ne "") {
        print $result . "\n";
    #}
}

if (STDIN->error) {
    print STDERR "Error: could not read stdin: $!\n";
    exit 1;
}
