# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

BEGIN {
seconds_per_day = 24*60*60
seconds_per_hour = 60*60
seconds_per_minute = 60

# maximum number of digits to print after the decimal point of the seconds value
# print the result if the rounded elapsed time is zero seconds
print_zero_seconds = 0
}


function abs(x) {
return x < 0.0 ? -x : x
}


function split_seconds(seconds,    days, hours, minutes, none_printed_yet, precision, result, seconds_str) {

seconds = abs(seconds)

days = 0
hours = 0
minutes = 0

none_printed_yet = 1
result = ""

precision = 0

if (seconds < 1000) {
    precision += 1
    if (seconds < 100) {
        precision += 1
        if (seconds < 10) {
            precision += 1
        }
    }
}

if (seconds >= seconds_per_day) {
days = int(seconds / seconds_per_day)
seconds = seconds % seconds_per_day
}

if (seconds >= seconds_per_hour) {
hours = int(seconds / seconds_per_hour)
seconds = seconds % seconds_per_hour
}

if (seconds >= seconds_per_minute) {
minutes = int(seconds / seconds_per_minute)
seconds = seconds % seconds_per_minute
}

# round seconds to precision digits after the decimal point
#seconds = int(seconds * seconds_num_digits_multiplier + 0.5) / seconds_num_digits_multiplier

seconds_str = sprintf("%.*f", precision, seconds)

# Rounding can reach 60 seconds, so carry it into the larger units.
if (substr(seconds_str, 1, 2) == "60") {
    seconds = 0
    if (++minutes == seconds_per_minute) {
        minutes = 0
        if (++hours == 24) {
            hours = 0
            ++days
        }
    }
}

if (days != 0) {
    if (none_printed_yet) {
        result = days "d"
    }
    else {
        result = result " " days "d"
    }
    none_printed_yet = 0

    # slower
    #if (length(result) > 0) {
    #   result = result " " days "d"
    #}
    #else {
    #   result = days "d"
    #}
}

if (hours != 0) {
    if (none_printed_yet) {
        result = hours "h"
    }
    else {
        result = result " " hours "h"
    }
    none_printed_yet = 0

    # slower
    #if (length(result) > 0) {
    #   result = result " " hours "h"
    #}
    #else {
    #   result = hours "h"
    #}
}

if (minutes != 0) {
    if (none_printed_yet) {
        result = minutes "m"
    }
    else {
        result = result " " minutes "m"
    }
    none_printed_yet = 0

    # slower
    #if (length(result) > 0) {
    #   result = result " " minutes "m"
    #}
    #else {
    #   result = minutes "m"
    #}
}

if (seconds != 0 || (print_zero_seconds && none_printed_yet)) {

    if (none_printed_yet) {
        #result = sprintf("%.*g", precision, seconds) "s"
        result = seconds_str "s"
    }
    else {
        #result = result " " sprintf("%.*g", precision, seconds) "s"
        result = result " " seconds_str "s"
    }
    none_printed_yet = 0

    # slower
    #if (length(result) > 0) {
    #   result = result " " sprintf("%.*g", precision, seconds) "s"
    #}
    #else {
    #   result = sprintf("%.*g", precision, seconds) "s"
    #}

    #none_printed_yet = 0

    #result = sprintf("%s%s%.*g%s", result, length(result) > 0? " ": "", precision, seconds, "s")
}

return result
}

NF==0 {
    print
}

NF>0 {
seconds = $1
#printf("# seconds=%.10f\n", seconds) #####
result = split_seconds(seconds)
#if (result != "") {
    print result
#}
}

