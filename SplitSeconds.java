// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: MPL-2.0

/*

javac SplitSeconds.java && d SplitSeconds.class

*/

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.StringJoiner;

class SplitSeconds
{
    private static final int seconds_per_day = 60 * 60 * 24;
    private static final int seconds_per_hour = 60 * 60;
    private static final int seconds_per_minute = 60;

    private static final boolean print_zero_seconds = true;

    // Whole seconds stop being exact in a double above this value.
    private static final double max_seconds = 0x1p+53;

    // Double.parseDouble also accepts hex, NaN, Infinity, and a d or f suffix.  All but hex end
    // in a letter, and hex needs an x.
    private static boolean may_parse(String line) {
        char last = line.charAt(line.length() - 1);
        return (last == '.' || (last >= '0' && last <= '9'))
            && line.indexOf('x') < 0 && line.indexOf('X') < 0;
    }

    private static String split_seconds(double seconds) {

        StringJoiner result = new StringJoiner(" ");

        // main rejects negative values, so this only clears the sign of -0.
        seconds = Math.abs(seconds);

        long seconds_int = (long)seconds;
        double seconds_frac = seconds - seconds_int;

        long days = 0;
        long hours = 0;
        long minutes = 0;

        int precision = 0;

        if (seconds_int < 1000) {
            precision += 1;
            if (seconds_int < 100) {
                precision += 1;
                if (seconds_int < 10) {
                    precision += 1;
                }
            }
        }

        if (seconds_int >= seconds_per_day) {
            days = seconds_int / seconds_per_day;
            seconds_int = seconds_int % seconds_per_day;
        }

        if (seconds_int >= seconds_per_hour) {
            hours = seconds_int / seconds_per_hour;
            seconds_int = seconds_int % seconds_per_hour;
        }

        if (seconds_int >= seconds_per_minute) {
            minutes = seconds_int / seconds_per_minute;
            seconds_int = seconds_int % seconds_per_minute;
        }

        seconds = seconds_int + seconds_frac;

        // DecimalFormat removes trailing zeros unless
        // setMinimumFractionDigits(precision) is also called.
        /*
        DecimalFormat df = new DecimalFormat("0");
        df.setMaximumFractionDigits(precision);
        df.setDecimalSeparatorAlwaysShown(false);
        var seconds_str = df.format(seconds);
        */

        // String.format rounds the shortest decimal form half-up, but printf
        // rounds the exact value half-even.
        //String seconds_str = String.format("%." + precision + "f", seconds);

        BigDecimal bd = new BigDecimal(seconds).setScale(precision, RoundingMode.HALF_EVEN);

        String seconds_str = bd.toPlainString();

        // Rounding can reach 60 seconds, so carry it into the larger units.
        if (seconds_str.startsWith("60")) {
            seconds = 0;
            if (++minutes == seconds_per_minute) {
                minutes = 0;
                if (++hours == 24) {
                    hours = 0;
                    ++days;
                }
            }
        }

        // Seconds that round to zero count as zero, so they don't print beside larger units.
        if (bd.signum() == 0) {
            seconds = 0;
        }

        if (days != 0) {
            result.add(String.valueOf(days) + "d");
        }

        if (hours != 0) {
            result.add(String.valueOf(hours) + "h");
        }

        if (minutes != 0) {
            result.add(String.valueOf(minutes) + "m");
        }

        if (seconds != 0 || (print_zero_seconds && result.length() == 0)) {
            result.add(seconds_str + "s");
        }

        return result.toString();
    }

    public static void main(String[] args) throws java.io.IOError
    {
        try {
            String line = null;

            while ((line = IO.readln()) != null)
            {
                line = line.strip();

                if (line.isEmpty()) {
                    IO.println();
                    continue;
                }

                double seconds = 0;

                try {
                    if (!may_parse(line)) {
                        throw new NumberFormatException();
                    }
                    seconds = Double.parseDouble(line);
                } catch (NumberFormatException e) {
                    System.err.println("Error: not a number: " + line);
                    System.exit(1);
                }

                if (!(seconds >= 0 && seconds <= max_seconds)) {
                    System.err.println("Error: out of range: " + line);
                    System.exit(1);
                }

                IO.println(split_seconds(seconds));
            }
        } catch (java.io.IOError e) {
            System.err.println("Error: could not read stdin: " + e.getCause().getMessage());
            System.exit(1);
        }
    }
}
