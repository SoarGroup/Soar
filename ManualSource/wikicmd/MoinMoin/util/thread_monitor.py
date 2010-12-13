# -*- coding: ascii -*-
"""
    Thread monitor - Check the state of all threads.

    Just call activate_hook() as early as possible in program execution.
    Then you can trigger the output of tracebacks of all threads
    by calling trigger_dump().

    Usage of Python 2.5 is recommended because it allows for a much safer
    and faster frame extraction.

    @copyright: 2006 Alexander Schremmer <alex AT alexanderweb DOT de>
    @license: GNU GPL Version 2
"""


__all__ = "activate_hook trigger_dump dump_regularly".split()


import sys
import threading
import traceback
from time import sleep
from StringIO import StringIO

from MoinMoin.support.python_compatibility import set


class AbstractMonitor(object):
    def activate_hook(self):
        """ Activates the thread monitor hook. Note that this might interfere
        with any kind of profiler and some debugging extensions. """
        raise NotImplementedError

    def trigger_dump(self, dumpfile=None):
        """ Triggers the dump of the tracebacks of all threads.
            If dumpfile is specified, it is used as the output file. """
        raise NotImplementedError

    def hook_enabled(self):
        """ Returns true if the thread_monitor hook is enabled. """
        raise NotImplementedError

class LegacyMonitor(AbstractMonitor):
    # global state
    dumping = False
    dump_file = None
    dumped = set()
    to_dump = set()
    hook_enabled = False

    def dump(cls, label):
        df = cls.dump_file or sys.stderr
        s = StringIO()
        print >>s, "\nDumping thread %s:" % (label, )
        try:
            raise ZeroDivisionError
        except ZeroDivisionError:
            f = sys.exc_info()[2].tb_frame.f_back.f_back
        traceback.print_list(traceback.extract_stack(f, None), s)
        df.write(s.getvalue())
    dump = classmethod(dump)

    def dump_hook(cls, a, b, c): # arguments are ignored
        if cls.dumping and sys.exc_info()[0] is None:
            thread = threading.currentThread()
            if thread in cls.to_dump:
                cls.dump(repr(thread))
                cls.to_dump.discard(thread)
                cls.dumped.add(thread)
                if not cls.to_dump:
                    cls.dumping = False
    dump_hook = classmethod(dump_hook)

    def trigger_dump(cls, dumpfile=None):
        cls.to_dump = set(threading.enumerate())
        if dumpfile is not None:
            cls.dump_file = dumpfile
        cls.dumping = True
    trigger_dump = classmethod(trigger_dump)

    def activate_hook(cls):
        sys.setprofile(cls.dump_hook)
        threading.setprofile(cls.dump_hook)
        cls.hook_enabled = True
    activate_hook = classmethod(activate_hook)

    def hook_enabled(cls):
        return cls.hook_enabled
    hook_enabled = classmethod(hook_enabled)


class DirectMonitor(AbstractMonitor):
    def __init__(self):
        self.enabled = False
        assert hasattr(sys, "_current_frames")

    def activate_hook(self):
        self.enabled = True

    def trigger_dump(self, dumpfile=None):
        if not self.enabled:
            return
        dumpfile = dumpfile or sys.stderr
        cur_frames = sys._current_frames()
        for i in cur_frames:
            s = StringIO()
            print >>s, "\nDumping thread (id %s):" % (i, )
            traceback.print_stack(cur_frames[i], file=s)
            dumpfile.write(s.getvalue())

    def hook_enabled(self):
        return self.enabled


def dump_regularly(seconds):
    """ Dumps the tracebacks every 'seconds' seconds. """
    activate_hook()

    def background_dumper(seconds):
        while 1:
            sleep(seconds)
            trigger_dump()

    threading.Thread(target=background_dumper, args=(seconds, )).start()


# Python 2.5 provides an optimised version
if hasattr(sys, "_current_frames"):
    mon = DirectMonitor()
else:
    mon = LegacyMonitor()

activate_hook = mon.activate_hook
trigger_dump = mon.trigger_dump
hook_enabled = mon.hook_enabled
