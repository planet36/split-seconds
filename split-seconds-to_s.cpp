// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: MPL-2.0

/*

gpp $OPTIMIZE_OPTIONS split-seconds-to_s.cpp -o split-seconds-to_s && d ./split-seconds-to_s

*/

#include "f_to_s.hpp"
#include "i_to_s.hpp"
#include "getline.hpp"

#include <cctype>
#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstring>
#include <print>
#include <string>
#include <system_error>

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

constexpr bool print_zero_seconds = true;

/*
This is the longest possible output:

9007199254713599 ->
104249991373d 23h 59m 59s
(size=25)

A value with days has no decimal places because precision is 0 from 1000 seconds up.
*/

/*
** Whole seconds stop being exact in a double above this value.
*/
constexpr double max_seconds = 0x1p+53;

constexpr size_t max_result_size = 32;

std::string split_seconds(double seconds)
{
    std::string result;
    result.reserve(max_result_size);

    // main rejects negative values, so this only clears the sign of -0.
    seconds = std::abs(seconds);

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

    std::string seconds_str = f_to_s(seconds, std::chars_format::fixed, precision);

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

    // Seconds that round to zero count as zero, so they don't print beside larger units.
    if (seconds_str.find_first_not_of("0.") == std::string::npos)
        seconds = 0;

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

        result.append(seconds_str);
        result.push_back('s');
    }

    return result;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::string line;
    while (getline(line, stdin))
    {
        const char* first = line.data();
        const char* last = first + line.size();

        while (first != last && std::isspace(static_cast<unsigned char>(*first)))
            ++first;
        while (first != last && std::isspace(static_cast<unsigned char>(last[-1])))
            --last;

        if (first == last)
        {
            std::println("");
            continue;
        }

        // from_chars doesn't accept a leading plus sign.
        if (*first == '+' && last - first > 1 && first[1] != '-')
            ++first;

        double seconds = 0;
        const auto [ptr, ec] = std::from_chars(first, last, seconds);

        if (ec != std::errc{} || ptr != last)
        {
            std::println(stderr, "Error: not a number: {}", line);
            return 1;
        }

        // This also rejects NaN because every comparison with it is false.
        if (!(seconds >= 0 && seconds <= max_seconds))
        {
            std::println(stderr, "Error: out of range: {}", line);
            return 1;
        }

        std::println("{}", split_seconds(seconds));
    }

    if (!feof(stdin))
    {
        std::println(stderr, "Error: could not read stdin: {}", std::strerror(errno));
        return 1;
    }

    return 0;
}
