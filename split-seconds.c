// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: MPL-2.0

/*

gccc $OPTIMIZE_OPTIONS split-seconds.c -o split-seconds-C -lm && d ./split-seconds-C

*/

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void my_div_f(const double x, const double y, double* quo, double* rem)
{
    *quo = trunc(x / y);
    //*rem = fmod(x, y);
    *rem = x - *quo * y;
}

void my_div_i(const int x, const int y, int* quo, int* rem)
{
    *quo = (x / y);
    *rem = (x % y);
}

constexpr int seconds_per_day = 60 * 60 * 24;
constexpr int seconds_per_hour = 60 * 60;
constexpr int seconds_per_minute = 60;

constexpr bool print_zero_seconds = true;

/*
This is the longest possible output:

9007199254713599 ->
104249991373d 23h 59m 59s
(size=25)

A value with days has no decimal places, because precision is 0 from 1000 seconds up.
*/

/*
** Whole seconds stop being exact in a double above this value.
*/
constexpr double max_seconds = 0x1p+53;

constexpr size_t max_result_size = 32;

void split_seconds(double seconds, char* result, const size_t result_size)
{
    memset(result, 0, result_size);

    // main rejects negative values, so this only clears the sign of -0.
    seconds = fabs(seconds);

    int precision = 0;

    if (seconds < 1000)
    {
        ++precision;
        if (seconds < 100)
        {
            ++precision;
            if (seconds < 10)
            {
                ++precision;
            }
        }
    }

    long int days = 0;
    int hours = 0;
    int minutes = 0;

    if (seconds >= seconds_per_day)
    {
        double quo = 0;
        double rem = 0;

        my_div_f(seconds, seconds_per_day, &quo, &rem);
        days = (long int)(quo);
        seconds = rem;
    }

    int seconds_int = (int)seconds;
    const double seconds_frac = seconds - seconds_int;

    if (seconds_int >= seconds_per_hour)
    {
        int rem = 0;

        my_div_i(seconds_int, seconds_per_hour, &hours, &rem);
        seconds_int = rem;
    }

    if (seconds_int >= seconds_per_minute)
    {
        int rem = 0;

        my_div_i(seconds_int, seconds_per_minute, &minutes, &rem);
        seconds_int = rem;
    }

    seconds = seconds_int + seconds_frac;

    char seconds_str[sizeof(" 10.000s")] = {};
    (void)snprintf(seconds_str, sizeof(seconds_str), "%.*f", precision, seconds);

    // Rounding can reach 60 seconds, so carry it into the larger units.
    if (strncmp(seconds_str, "60", 2) == 0)
    {
        seconds = 0;
        if (++minutes == seconds_per_minute)
        {
            minutes = 0;
            if (++hours == 24)
            {
                hours = 0;
                ++days;
            }
        }
    }

    // Seconds that round to zero count as zero, so they don't print beside larger units.
    if (strspn(seconds_str, "0.") == strlen(seconds_str))
        seconds = 0;

    bool none_printed_yet = true;

    char days_buf[sizeof("104249991374d")] = {};
    char hours_buf[sizeof(" 10h")] = {};
    char minutes_buf[sizeof(" 10m")] = {};
    char seconds_buf[sizeof(" 10.000s")] = {};

    if (days != 0)
    {
        (void)snprintf(days_buf, sizeof(days_buf), "%s%ldd", (!none_printed_yet ? " " : ""), days);
        none_printed_yet = false;
    }

    if (hours != 0)
    {
        (void)snprintf(hours_buf, sizeof(hours_buf), "%s%dh", (!none_printed_yet ? " " : ""), hours);
        none_printed_yet = false;
    }

    if (minutes != 0)
    {
        (void)snprintf(minutes_buf, sizeof(minutes_buf), "%s%dm", (!none_printed_yet ? " " : ""), minutes);
        none_printed_yet = false;
    }

    if (seconds != 0 || (print_zero_seconds && none_printed_yet))
    {
        (void)snprintf(seconds_buf, sizeof(seconds_buf), "%s%ss", (!none_printed_yet ? " " : ""), seconds_str);
        none_printed_yet = false;
    }

    (void)snprintf(result, result_size, "%s%s%s%s", days_buf, hours_buf, minutes_buf, seconds_buf);
}

#define nl (void)putchar('\n')

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    char* line = NULL;
    size_t n = 0;
    while (getline(&line, &n, stdin) >= 0)
    {
        char* first = line;

        while (isspace((unsigned char)*first))
            ++first;

        char* last = first + strlen(first);

        while (last != first && isspace((unsigned char)last[-1]))
            --last;

        *last = '\0';

        if (first == last)
        {
            nl;
            continue;
        }

        // strtod also accepts hex, NaN, and infinity, and those need other characters.
        char* end = NULL;
        const double seconds = (strspn(first, "0123456789+-.eE") == strlen(first)) ? strtod(first, &end) : 0;

        if (end != last)
        {
            fprintf(stderr, "Error: not a number: %s\n", first);
            return 1;
        }

        if (!(seconds >= 0 && seconds <= max_seconds))
        {
            fprintf(stderr, "Error: out of range: %s\n", first);
            return 1;
        }

        char result[max_result_size];

        //printf("# seconds=%.10f\n", seconds); //#####
        split_seconds(seconds, result, sizeof(result));
        puts(result);
    }

    if (!feof(stdin))
    {
        fprintf(stderr, "Error: could not parse stdin\n");
        return 1;
    }

    return 0;
}
