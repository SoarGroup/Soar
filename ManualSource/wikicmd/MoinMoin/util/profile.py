""" profile - moin profiling utilities

This module provides profilers used to profile the memory usage of a
long running process.

Typical usage:

 1. Create a profiler:

    from MoinMoin.util.profile import Profiler
    profiler = Profiler('my log')

 2. In the request handler, add each request to the profiler:

    profiler.addRequest()

 3. If you like, you can add extra samples:

    profiler.sample()

You can customize the profiler when you create it:

 * requestsPerSample (default 100):

   How many requests to run between samples. Set higher for live wiki or
   lower for more accurate results.

 * collect (default 0):

   Use gc.collect to force a memory cleanup each sample. Keeps the
   memory usage much lower, but your profile data will not reflect the
   real world memory usage of the application.

Based on code by Oliver Graf

@copyright: 2004 Nir Soffer
@license: GNU GPL, see COPYING for details.
"""

import os, time, gc


class Profiler:
    """ Profile memory usage

    Profiler count requests and sample memory usage.

    FIXME: We might want to save the profiler log in the profiled wiki
    data dir, but the data dir is available only later in request. This
    should be fixed by loading the config earlier.
    """
    def __init__(self, name, requestsPerSample=100, collect=0):
        """ Initialize a profiler

        @param name: profiler name, used in the log file name
        @param requestsPerSample: how many request to run between samples
        @param collect: should call gc.collect() in each sample
        """
        logname = '%s--%s.log' % (name, time.strftime('%Y-%m-%d--%H-%M'))
        self.logfile = file(logname, 'a')
        self.requestsPerSample = requestsPerSample
        self.collect = collect
        self.pid = os.getpid()
        self.count = 0 # count between somples
        self.requests = 0 # requests added
        self.data = {'collect': 'NA'} # Sample data

    def addRequest(self):
        """ Add a request to the profile

        Call this for each page request.

        WARNING: This is the most important call. if you don't call this
        for each request - you will not have any profile data.

        Invoke sample when self.count reach self.requestsPerSample.
        """
        self.requests += 1
        self.count += 1
        if self.count == self.requestsPerSample:
            # Time for a sample
            self.count = 0
            self.sample()

    def sample(self):
        """ Make a sample of memory usage and log it

        You can call this to make samples between the samples done each
        requestsPerSample, for example, at startup.

        Invoke common methods for all profilers. Some profilers like
        TwistedProfiler override this method.
        """
        self._setData()
        self._setMemory()
        self._log()

    # Private methods ------------------------------------------------------

    def _setData(self):
        """ Collect sample data into self.data

        Private method used by profilers.
        """
        d = self.data
        d['date'] = time.strftime('%Y-%m-%d %H:%M:%S')
        d['requests'] = self.requests
        if self.collect:
            d['collect'] = str(gc.collect())
        d['objects'] = len(gc.get_objects())
        d['garbage'] = len(gc.garbage)

    def _setMemory(self):
        """ Get process memory usage

        Private method used by profilers.

        Uses ps call, maybe we should use procfs on Linux or maybe
        getrusage system call (using the ctypes module).
        """
        lines = os.popen('/bin/ps -p %s -o rss' % self.pid).readlines()
        self.data['memory'] = lines[1].strip()

    def _log(self):
        """ Format sample and write to log

        Private method used by profilers.
        """
        line = ('%(date)s req:%(requests)d mem:%(memory)sKB collect:%(collect)s '
                'objects:%(objects)d garbage:%(garbage)d\n' % self.data)
        self.logfile.write(line)
        self.logfile.flush()


class TwistedProfiler(Profiler):
    """ Twisted specific memory profiler

    Customize the way we call ps, to overcome blocking problems on
    twisted.
    """

    def __init__(self, name, requestsPerSample=100, collect=0):
        """ Initialized twisted profiler

        Invoke Profiler.__init__ and import getProcessOuput from
        twisted.
        """
        Profiler.__init__(self, name, requestsPerSample, collect)
        from twisted.internet.utils import getProcessOutput
        self._getProcessOutput = getProcessOutput

    def sample(self):
        """ Make a sample of memory usage and log it

        On twisted we can't just call ps - we have to use deferred,
        which will call us using a callback when its finished, and then
        we log.

        Since twisted continue to serve while the deferred fetch the
        memory, the reading may be late in few requests.
        """
        self._setData()
        # Memory will be available little later
        deferred = self._getProcessOutput('/bin/ps',
                                          ('-p', str(self.pid), '-o', 'rss'))
        deferred.addCallback(self._callback)

    # Private methods ------------------------------------------------------

    def _callback(self, data):
        """ Called from deferred when ps output is available

        Private method, don't call this.
        """
        self.data['memory'] = data.split('\n')[1].strip()
        self._log()


if __name__ == '__main__':
    # In case someone try to run as a script
    print __doc__

