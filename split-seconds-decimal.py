# SPDX-FileCopyrightText: Steven Ward
# SPDX-License-Identifier: MPL-2.0

import sys
from decimal import Decimal as D, InvalidOperation

seconds_per_day = 60 * 60 * 24
seconds_per_hour = 60 * 60
seconds_per_minute = 60

print_zero_seconds = True

# This matches the limit of the implementations that use a double.
max_seconds = D(2) ** 53

def split_seconds(seconds: D) -> str:

    result = []

    # The caller rejects negative values, so this only clears the sign of -0.
    seconds = abs(seconds)

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

    # Round seconds to precision digits after the decimal point.
    ##### round does round half even.
    #seconds = round(seconds_int + seconds_frac, precision)

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


def stdin_lines():

    # This covers reading only because errors in the caller's loop don't pass through here.
    try:
        yield from sys.stdin
    except OSError as e:
        print(f'Error: could not read stdin: {e.strerror}', file=sys.stderr)
        sys.exit(1)


for line in stdin_lines():
    line = line.strip()

    if not line:  # empty
        print()
        continue

    # Decimal also accepts underscores and non-ASCII digits.  The range check below handles
    # NaN and infinity.
    try:
        seconds = D(line)
    except InvalidOperation:
        seconds = None

    if seconds is None or '_' in line or not line.isascii():
        print(f'Error: not a number: {line}', file=sys.stderr)
        sys.exit(1)

    if not seconds.is_finite() or not 0 <= seconds <= max_seconds:
        print(f'Error: out of range: {line}', file=sys.stderr)
        sys.exit(1)

    #print('# seconds={:.10f}'.format(seconds)) #####
    print(split_seconds(seconds))
