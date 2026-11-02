// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: MPL-2.0

/*

gpp $OPTIMIZE_OPTIONS split-seconds-google.cpp -o split-seconds-google -ldouble-conversion && d ./split-seconds-google

*/

#include "getline.hpp"

#include <double-conversion/double-conversion.h>

#include <cerrno>
#include <cmath>
#include <cstring>
#include <print>
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

constexpr bool print_zero_seconds = true;

// https://github.com/google/double-conversion/blob/master/double-conversion/string-to-double.h
// https://github.com/google/double-conversion/blob/master/double-conversion/double-to-string.h

constexpr int s_to_d_flags =
    double_conversion::StringToDoubleConverter::ALLOW_LEADING_SPACES|
    double_conversion::StringToDoubleConverter::ALLOW_TRAILING_SPACES|
    0
    ;

constexpr int d_to_s_flags = double_conversion::DoubleToStringConverter::NO_FLAGS;

constexpr double empty_string_value = 0.0;
constexpr double junk_string_value = NAN;
const char* infinity_symbol = "inf";
const char* nan_symbol = "nan";
constexpr char exponent_character = 'e';
constexpr int decimal_in_shortest_low = -6; // -20
constexpr int decimal_in_shortest_high = 21; // 20
constexpr int max_leading_padding_zeroes_in_precision_mode = 6; // 20
constexpr int max_trailing_padding_zeroes_in_precision_mode = 1; // 20
constexpr int min_exponent_width = 2;

const auto s_to_d = double_conversion::StringToDoubleConverter(
        s_to_d_flags,
        empty_string_value,
        junk_string_value,
        nullptr,
        nullptr);

const auto d_to_s = double_conversion::DoubleToStringConverter(
        d_to_s_flags,
        infinity_symbol,
        nan_symbol,
        exponent_character,
        decimal_in_shortest_low,
        decimal_in_shortest_high,
        max_leading_padding_zeroes_in_precision_mode,
        max_trailing_padding_zeroes_in_precision_mode,
        min_exponent_width);

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

    char tmp[max_result_size] = {'\0'};
    double_conversion::StringBuilder builder(tmp, sizeof(tmp));

    // ToFixed rounds exact ties up, not half-even like printf, so move a tie that
    // should round down to just below it.
    constexpr double scales[] = {1.0, 10.0, 100.0, 1000.0};
    const double scale = scales[precision];
    const double scaled = seconds * scale;
    const double scaled_floor = std::floor(scaled);

    if (scaled - scaled_floor == 0.5 &&
        std::fma(seconds, scale, -scaled) == 0.0 &&
        std::fmod(scaled_floor, 2.0) == 0.0)
    {
        seconds = std::nextafter(seconds, 0.0);
    }

    //d_to_s.ToPrecision(seconds, precision, &builder);
    d_to_s.ToFixed(seconds, precision, &builder);

    std::string seconds_str = builder.Finalize();

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
        result.append(std::to_string(days));
        result.push_back('d');
    }

    if (hours != 0)
    {
        if (!result.empty())
            result.push_back(' ');
        result.append(std::to_string(hours));
        result.push_back('h');
    }

    if (minutes != 0)
    {
        if (!result.empty())
            result.push_back(' ');
        result.append(std::to_string(minutes));
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
        // The converter returns empty_string_value for a blank line.
        if (line.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
        {
            std::println("");
            continue;
        }

        int processed_count = 0;
        const double seconds = s_to_d.StringToDouble(line.c_str(), static_cast<int>(line.size()), &processed_count);

        if (std::isnan(seconds))
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
