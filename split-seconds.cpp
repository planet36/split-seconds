// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: MPL-2.0

/*

gpp $OPTIMIZE_OPTIONS split-seconds.cpp -o split-seconds && d ./split-seconds

*/

#include "getline.hpp"

#include <cmath>
#include <print>
#include <sstream>
#include <string>

void my_div_f(const double x, const double y, double& quo, double& rem)
{
    quo = std::trunc(x / y);
    //rem = std::fmod(x, y);
    rem = x - quo * y;
}

void my_div_i(const int x, const int y, int& quo, int& rem)
{
    quo = (x / y);
    rem = (x % y);
}

constexpr int seconds_per_day = 60 * 60 * 24;
constexpr int seconds_per_hour = 60 * 60;
constexpr int seconds_per_minute = 60;

constexpr bool print_zero_seconds = false;

/*
This is the longest known output:

0x1.7caa73db624d1p+78 ->
5201514925802699776d 10012h 26m 40s
(size=35)

The hours exceed 23 because my_div_f rounds quo at this magnitude.  The length
depends on FMA contraction of its remainder, and without that, the longest known
output has size=34.  A value with days has no decimal places, because precision
is 0 from 1000 seconds up.
*/

/*
** 796899343984252562702335 == 0x1.517ffffffffffp+79
** 796899343984252562702336 == 0x1.518p+79
*/
constexpr double max_seconds = 0x1.517ffffffffffp+79;

//constexpr size_t max_result_size = 32;

std::string split_seconds(double seconds)
{
    seconds = std::abs(seconds);

    if (seconds > max_seconds)
        seconds = max_seconds;

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

        my_div_f(seconds, seconds_per_day, quo, rem);
        days = static_cast<long int>(quo);
        seconds = rem;
    }

    int seconds_int = static_cast<int>(seconds);
    const double seconds_frac = seconds - seconds_int;

    //if (seconds >= seconds_per_hour)
    if (seconds_int >= seconds_per_hour)
    {
        //int quo = 0;
        int rem = 0;

        //my_div_f(seconds, seconds_per_hour, quo, rem);
        my_div_i(seconds_int, seconds_per_hour, hours, rem);
        //hours = quo;
        seconds_int = rem;
    }

    //if (seconds >= seconds_per_minute)
    if (seconds_int >= seconds_per_minute)
    {
        //int quo = 0;
        int rem = 0;

        //my_div_f(seconds, seconds_per_minute, quo, rem);
        my_div_i(seconds_int, seconds_per_minute, minutes, rem);
        //minutes = quo;
        seconds_int = rem;
    }

    seconds = seconds_int + seconds_frac;

    std::ostringstream oss;

    //oss << std::defaultfloat;
    oss << std::fixed;
    oss.precision(precision);
    oss << seconds;
    const std::string seconds_str = oss.str();
    oss.str("");
    //oss.precision(precision_orig);

    // Rounding can reach 60 seconds, so carry it into the larger units.
    if (seconds_str.starts_with("60"))
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

    bool none_printed_yet = true;

    if (days != 0)
    {
        if (!none_printed_yet)
            oss << ' ';
        oss << days << 'd';
        none_printed_yet = false;
    }

    if (hours != 0)
    {
        if (!none_printed_yet)
            oss << ' ';
        oss << hours << 'h';
        none_printed_yet = false;
    }

    if (minutes != 0)
    {
        if (!none_printed_yet)
            oss << ' ';
        oss << minutes << 'm';
        none_printed_yet = false;
    }

    if (seconds != 0 || (print_zero_seconds && none_printed_yet))
    {
        if (!none_printed_yet)
            oss << ' ';

        oss << seconds_str << 's';
        //none_printed_yet = false;
    }

    return oss.str();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::string line;
    while (getline(line, stdin))
    {
        double seconds = 0;

        const int scanf_ret = sscanf(line.c_str(), "%lf\n", &seconds);

        if (scanf_ret == EOF || scanf_ret != 1)
        {
            std::println("");
            continue;
        }

        if (!std::isfinite(seconds))
        {
            std::println(stderr, "Error: not a finite number: {}", seconds);
            return 1;
        }

        std::println("{}", split_seconds(seconds));
    }

    if (!feof(stdin))
    {
        std::println(stderr, "Error: could not parse stdin");
        return 1;
    }

    return 0;
}
