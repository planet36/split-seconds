# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

import sys
from decimal import Decimal as D, ROUND_HALF_UP

seconds_per_day = 60 * 60 * 24
seconds_per_hour = 60 * 60
seconds_per_minute = 60

print_zero_seconds = True

# 796899343984252562702335 == 0x1.517ffffffffffp+79
# 796899343984252562702336 == 0x1.518p+79
#max_seconds = D.from_float(float.fromhex('0x1.517ffffffffffp+79'))
max_seconds = D('796899343984252562702335')
#max_seconds = 796899343984252562702335

quantize_exp = [
        D(10) ** -0,
        D(10) ** -1,
        D(10) ** -2,
        D(10) ** -3,
        ]

def split_seconds(seconds: D) -> str:

    result = []

    seconds = abs(seconds)

    if seconds > max_seconds:
        seconds = max_seconds

    seconds_int = int(seconds)
    seconds_frac = seconds - seconds_int

    days = 0
    hours = 0
    minutes = 0

    precision = 0

    if seconds_int < 1000:
        precision += 1
        if seconds_int < 100:
            precision += 1
            if seconds_int < 10:
                precision += 1

    if seconds_int >= seconds_per_day:
        (days, seconds_int) = divmod(seconds_int, seconds_per_day)
    if seconds_int >= seconds_per_hour:
        (hours, seconds_int) = divmod(seconds_int, seconds_per_hour)
    if seconds_int >= seconds_per_minute:
        (minutes, seconds_int) = divmod(seconds_int, seconds_per_minute)

    # round seconds to precision digits after the decimal point
    ##### round does round half even
    #seconds = round(seconds_int + seconds_frac, precision)
    #seconds = (seconds_int + seconds_frac).quantize(quantize_exp[precision], rounding=ROUND_HALF_UP)

    seconds = seconds_int + seconds_frac

    seconds_str = f'{seconds:.{precision}f}'

    # Rounding can reach 60 seconds, so carry it into the larger units.
    if seconds_str.startswith('60'):
        seconds = 0
        minutes += 1
        if minutes == seconds_per_minute:
            minutes = 0
            hours += 1
            if hours == 24:
                hours = 0
                days += 1

    # Seconds that round to zero count as zero, so they don't print beside larger units.
    if not seconds_str.strip('0.'):
        seconds = 0

    if days != 0:
        result.append(str(days) + 'd')

    if hours != 0:
        result.append(str(hours) + 'h')

    if minutes != 0:
        result.append(str(minutes) + 'm')

    if seconds != 0 or (print_zero_seconds and not result):
        #if '.' in seconds_str and seconds_str.endswith('0'):
        #   seconds_str = seconds_str.rstrip('0').rstrip('.')
        result.append(seconds_str + 's')

    return ' '.join(result)


for line in sys.stdin:
    line = line.strip()

    if not line: # empty
        print()
        continue

    seconds = D(line)

    #print('# seconds={:.10f}'.format(seconds)) #####
    print(split_seconds(seconds))
