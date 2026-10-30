# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

import sys

seconds_per_day = 60 * 60 * 24
seconds_per_hour = 60 * 60
seconds_per_minute = 60

print_zero_seconds = False

# 796899343984252562702335 == 0x1.517ffffffffffp+79
# 796899343984252562702336 == 0x1.518p+79
max_seconds = float.fromhex('0x1.517ffffffffffp+79')

'''
def dround(x: float, decimal_places: int):

    multiple_10 = pow(10, decimal_places);
    return round(x * multiple_10) / multiple_10;
'''

def split_seconds(seconds: float) -> str:

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
    #seconds = dround(seconds, precision)

    seconds = seconds_int + seconds_frac

    if days != 0:
        result.append(str(days) + 'd')

    if hours != 0:
        result.append(str(hours) + 'h')

    if minutes != 0:
        result.append(str(minutes) + 'm')

    if seconds != 0 or (print_zero_seconds and not result):
        seconds_str = f'{seconds:.{precision}f}'
        #if '.' in seconds_str and seconds_str.endswith('0'):
        #   seconds_str = seconds_str.rstrip('0').rstrip('.')
        result.append(seconds_str + 's')

    return ' '.join(result)


for line in sys.stdin:
    line = line.strip()

    if not line: # empty
        print()
        continue

    seconds = float(line)

    #print('# seconds={:.10f}'.format(seconds)) #####
    print(split_seconds(seconds))
