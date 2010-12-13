"""
    MoinMoin event log class

    The global event-log is mainly used for statistics (e.g. EventStats).

    @copyright: 2007 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

import time

from MoinMoin.logfile import LogFile
from MoinMoin import wikiutil

class EventLog(LogFile):
    """ The global event-log is mainly used for statistics (e.g. EventStats) """
    def __init__(self, request, filename=None, buffer_size=65536, **kw):
        if filename is None:
            rootpagename = kw.get('rootpagename', None)
            if rootpagename:
                from MoinMoin.Page import Page
                filename = Page(request, rootpagename).getPagePath('event-log', isfile=1)
            else:
                filename = request.rootpage.getPagePath('event-log', isfile=1)
        LogFile.__init__(self, filename, buffer_size)

    def add(self, request, eventtype, values=None, add_http_info=1,
            mtime_usecs=None):
        """ Write an event of type `eventtype, with optional key/value
            pairs appended (i.e. you have to pass a dict).
        """
        if request.isSpiderAgent:
            return

        if mtime_usecs is None:
            mtime_usecs = wikiutil.timestamp2version(time.time())

        if values is None:
            values = {}
        if request.cfg.log_remote_addr and add_http_info:
            # if cfg.log_remote_addr is False (usually for privacy reasons),
            # we likely do not want to log user agent and http referer either.
            for key in ['remote_addr', 'http_user_agent', 'http_referer']:
                value = getattr(request, key, '')
                if value:
                    # Save those http headers in UPPERcase
                    values[key.upper()] = value
        # Encode values in a query string TODO: use more readable format
        values = wikiutil.makeQueryString(values)
        self._add(u"%d\t%s\t%s\n" % (mtime_usecs, eventtype, values))

    def parser(self, line):
        """ parse a event-log line into its components """
        try:
            time_usecs, eventtype, kvpairs = line.rstrip().split('\t')
        except ValueError:
            # badly formatted line in file, skip it
            return None
        return long(time_usecs), eventtype, wikiutil.parseQueryString(kvpairs)

    def set_filter(self, event_types=None):
        """ optionally filter log for specific event types """
        if event_types is None:
            self.filter = None
        else:
            self.filter = lambda line: (line[1] in event_types)


