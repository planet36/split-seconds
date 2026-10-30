// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: MPL-2.0

/*

gpp $OPTIMIZE_OPTIONS split-seconds-to_s.cpp -o split-seconds-to_s && d ./split-seconds-to_s

*/

#include "f_to_s.hpp"
#include "i_to_s.hpp"
#include "getline.hpp"

#include <fmt/format.h>

#include <cmath>
#include <string>

/*
//##### XXX: only string_view has remove_suffix
void str_remove_suffix(std::string& s, const std::string::size_type n)
{
    if (n > s.size())
        s.clear();
    else
        s.erase(s.size() - n);
}
*/

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
Theoretical max length of output string:

??? ->
9223372036854775807d 23h 59m 59.999s
(size=36)

But this output might not even be possible.

0x1.517ffffffffffp+79 ->
9223372036854773760d
(size=20)

8640000086399.999 ->
100000000d 23h 59m 59.999s
(size=26)
*/

/*
** 796899343984252562702335 == 0x1.517ffffffffffp+79
** 796899343984252562702336 == 0x1.518p+79
*/
constexpr double max_seconds = 0x1.517ffffffffffp+79;

constexpr size_t max_result_size = 32;

std::string split_seconds(double seconds)
{
    std::string result;
    result.reserve(max_result_size);

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

    if (days != 0)
    {
        if (!result.empty())
            result.push_back(' ');
        result.append(i_to_s(days));
        result.push_back('d');
    }

    if (hours != 0)
    {
        if (!result.empty())
            result.push_back(' ');
        result.append(i_to_s(hours));
        result.push_back('h');
    }

    if (minutes != 0)
    {
        if (!result.empty())
            result.push_back(' ');
        result.append(i_to_s(minutes));
        result.push_back('m');
    }

    if (seconds != 0 || (print_zero_seconds && result.empty()))
    {
        if (!result.empty())
            result.push_back(' ');
        std::string seconds_str = f_to_s(seconds, std::chars_format::fixed, precision);

        /*
        if (seconds_str.contains('.'))
        {
            while (seconds_str.ends_with('0'))
                str_remove_suffix(seconds_str, 1);

            if (seconds_str.ends_with('.'))
                str_remove_suffix(seconds_str, 1);
        }
        */

        result.append(seconds_str);
        result.push_back('s');
    }

    return result;
}

#define nl (void)std::putchar('\n')

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::string line;
    while (getline(line, stdin))
    {
        double seconds = 0;

        const int scanf_ret = sscanf(line.c_str(), "%lf\n", &seconds);

        if (scanf_ret == EOF || scanf_ret != 1)
        {
            nl;
            continue;
        }

        if (!std::isfinite(seconds))
        {
            fmt::println(stderr, "Error: not a finite number: {}", seconds);
            return 1;
        }

        fmt::println("{}", split_seconds(seconds));
    }

    if (!feof(stdin))
    {
        fmt::println(stderr, "Error: could not parse stdin");
        return 1;
    }

    return 0;
}
