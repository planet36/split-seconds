# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

use strict;
use warnings;
use Scalar::Util qw(looks_like_number);

use constant SECONDS_PER_DAY => 24*60*60;
use constant SECONDS_PER_HOUR => 60*60;
use constant SECONDS_PER_MINUTE => 60;

# maximum number of digits to print after the decimal point of the seconds value
#use constant SECONDS_NUM_DIGITS => 2;
#use constant SECONDS_NUM_DIGITS_MULTIPLIER => 10**SECONDS_NUM_DIGITS;
# print the result if the rounded elapsed time is zero seconds
use constant PRINT_ZERO_SECONDS => 0;

sub split_seconds {

    my $seconds = $_[0];
    my $result = "";

    if (!looks_like_number($seconds)) {
        return $result;
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

    # round seconds to SECONDS_NUM_DIGITS digits after the decimal point
    #$seconds = int($seconds * SECONDS_NUM_DIGITS_MULTIPLIER + 0.5) / SECONDS_NUM_DIGITS_MULTIPLIER;

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
        $result .= sprintf("%.*f", $precision, $seconds) . "s";
    }

    return $result;
}

while (my $line = <STDIN>) {

    chomp($line);

    if ($line eq "") {
        print "\n";
        next;
    }

    my $seconds = 0;
    $seconds = $line;

    my $result = "";
    #printf("# seconds=%.10f\n", $seconds); #####
    $result = split_seconds($seconds);

    #if ($result ne "") {
        print $result . "\n";
    #}
}
