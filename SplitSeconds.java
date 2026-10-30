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

    private static final boolean print_zero_seconds = false;

    private static final double max_seconds = 0x1.517ffffffffffp+79;

    private static String split_seconds(double seconds) {

        StringJoiner result = new StringJoiner(" ");

        seconds = Math.abs(seconds);

        if (seconds > max_seconds) {
            seconds = max_seconds;
        }

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
            // XXX: removes trailing zeros (setMinimumFractionDigits(precision) keeps them)
            /*
            DecimalFormat df = new DecimalFormat("0");
            df.setMaximumFractionDigits(precision);
            df.setDecimalSeparatorAlwaysShown(false);
            var seconds_str = df.format(seconds);
            */

            // XXX: rounds the shortest decimal form half-up, not the exact value half-even like printf
            //String seconds_str = String.format("%." + precision + "f", seconds);

            BigDecimal bd = new BigDecimal(seconds).setScale(precision, RoundingMode.HALF_EVEN);

            String seconds_str = bd.toPlainString();

            result.add(seconds_str + "s");
        }

        return result.toString();
    }

    public static void main(String[] args) throws java.io.IOError
    {
        String line = null;

        while ((line = IO.readln()) != null)
        {
            line = line.strip();

            if (line.isEmpty()) {
                IO.println();
                continue;
            }

            double seconds = Double.parseDouble(line);

            IO.println(split_seconds(seconds));
        }
    }
}
