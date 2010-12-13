# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Clock

    @copyright: 2001-2003 Juergen Hermann <jh@web.de>,
                2003-2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

import time

class Clock:
    """ Helper class for code profiling
        we do not use time.clock() as this does not work across threads
        This is not thread-safe when it comes to multiple starts for one timer.
        It is possible to recursively call the start and stop methods, you
        should just ensure that you call them often enough :)
    """

    def __init__(self):
        self.timings = {}
        self.states = {}

    def _get_name(timer, generation):
        if generation == 0:
            return timer
        else:
            return "%s|%i" % (timer, generation)
    _get_name = staticmethod(_get_name)

    def start(self, timer):
        state = self.states.setdefault(timer, -1)
        new_level = state + 1
        name = Clock._get_name(timer, new_level)
        self.timings[name] = time.time() - self.timings.get(name, 0)
        self.states[timer] = new_level

    def stop(self, timer):
        state = self.states.setdefault(timer, -1)
        if state >= 0: # timer is active
            name = Clock._get_name(timer, state)
            self.timings[name] = time.time() - self.timings[name]
            self.states[timer] = state - 1

    def value(self, timer):
        base_timer = timer.split("|")[0]
        state = self.states.get(base_timer, None)
        if state == -1:
            result = "%.3fs" % self.timings[timer]
        elif state is None:
            result = "- (%s)" % state
        else:
            #print "Got state %r" % state
            result = "%.3fs (still running)" % (time.time() - self.timings[timer])
        return result

    def dump(self):
        outlist = []
        for timer in self.timings:
            value = self.value(timer)
            outlist.append("%s = %s" % (timer, value))
        outlist.sort()
        return outlist
