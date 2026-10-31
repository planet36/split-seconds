// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: MPL-2.0

/*

gpp $OPTIMIZE_OPTIONS               split-seconds-fmt.cpp -o split-seconds-FMT     && d ./split-seconds-FMT
gpp $OPTIMIZE_OPTIONS -DUSE_FMT_INT split-seconds-fmt.cpp -o split-seconds-FMT_INT && d ./split-seconds-FMT_INT

*/

#include "getline.hpp"

#include <fmt/format.h>

#include <cctype>
#include <charconv>
#include <cmath>
#include <string>
#include <system_error>

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

constexpr bool print_zero_seconds = true;

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

    std::string seconds_str = fmt::format("{:.{}f}", seconds, precision);

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

#ifdef USE_FMT_INT
    // use format_int() and append()
#else
    auto bii = std::back_inserter(result); // std::back_insert_iterator
#endif

    if (days != 0)
    {
        if (!result.empty())
            result.push_back(' ');
#ifdef USE_FMT_INT
        // fmt::to_string is slightly faster than std::to_string
        //result.append(fmt::to_string(days));

        result.append(fmt::format_int(days).str());
#else
        fmt::format_to(bii, "{}", days);
#endif
        result.push_back('d');
    }

    if (hours != 0)
    {
        if (!result.empty())
            result.push_back(' ');
#ifdef USE_FMT_INT
        // fmt::to_string is slightly faster than std::to_string
        //result.append(fmt::to_string(hours));

        result.append(fmt::format_int(hours).str());
#else
        fmt::format_to(bii, "{}", hours);
#endif
        result.push_back('h');
    }

    if (minutes != 0)
    {
        if (!result.empty())
            result.push_back(' ');
#ifdef USE_FMT_INT
        // fmt::to_string is slightly faster than std::to_string
        //result.append(fmt::to_string(minutes));

        result.append(fmt::format_int(minutes).str());
#else
        fmt::format_to(bii, "{}", minutes);
#endif
        result.push_back('m');
    }

    if (seconds != 0 || (print_zero_seconds && result.empty()))
    {
        if (!result.empty())
            result.push_back(' ');
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
            fmt::println("");
            continue;
        }

        // from_chars doesn't accept a leading plus sign.
        if (*first == '+' && last - first > 1 && first[1] != '-')
            ++first;

        double seconds = 0;
        const auto [ptr, ec] = std::from_chars(first, last, seconds);

        if (ec != std::errc{} || ptr != last)
        {
            fmt::println(stderr, "Error: not a number: {}", line);
            return 1;
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
