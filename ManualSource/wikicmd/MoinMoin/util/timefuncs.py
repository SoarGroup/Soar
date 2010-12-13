# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Date & Time Utilities

    @copyright: 2003 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

# we guarantee that time is always imported!
import re, time
from email.Utils import formatdate

def tmtuple(tmsecs=None):
    """ Return a time tuple.

        This is currently an alias for gmtime(), but allows later tweaking.
    """
    # avoid problems due to timezones etc. - especially a underflow
    if -86400 <= tmsecs <= 86400: # if we are around 0, we maybe had
        tmsecs = 0                # 0 initially, so reset it to 0.
    return time.gmtime(tmsecs or time.time())

def formathttpdate(tmsecs=None):
    """ Return a HTTP date/time stamp as defined in
        http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3 .
    """
    stamp = formatdate(tmsecs, False)
    # replace non-standard "-0000" at end with http-mandated "GMT"
    stamp = re.match('^(.*) [\-\+]0000$', stamp).group(1) + " GMT"
    return stamp

def W3CDate(tmsecs=None):
    """ Return UTC time string according to http://www.w3.org/TR/NOTE-datetime
    """
    if not tmsecs:
        tmsecs = time.gmtime()
    return time.strftime("%Y-%m-%dT%H:%M:%S", tmsecs) + "Z"

