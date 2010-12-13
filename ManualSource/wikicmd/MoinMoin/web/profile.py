# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - WSGI middlewares for profiling

    These have been ported from server_standalone to provide application
    profiling for a WSGI application. They are implemented as WSGI
    middlewares, so they can be plugged right in front of the MoinMoin
    WSGI application. Attention has to be payed, that at the end of
    profiling the `shutdown`-method has to be invoked, so that the
    middlewares can write the reports to the filesystem.

    TODO: in pre-WSGI MoinMoin those profilers where integrated in
          the standalone server and also some other gateway interfaces.
          In the near future the middlewares here could be again made
          configurable automatically with command line switches or
          wiki configuration options.

    @copyright: 2008 MoinMoin:FlorianKrupicka,
    @license: GNU GPL, see COPYING for details.
"""
from werkzeug import get_current_url

from MoinMoin import log
logging = log.getLogger(__name__)

class ProfilerMiddleware(object):
    """ Abstract base class for profiling middlewares.

    Concrete implementations of this class should provide implementations
    of `run_profile` and `shutdown`, the latter which should be called by
    the code utilizing the profiler.
    """
    def __init__(self, app):
        self.app = app

    def profile(self, environ, start_response):
        """
        Profile the request. Exceptions encountered during the profile are
        logged before being propagated for further handling.
        """
        method = environ.get('REQUEST_METHOD', 'GET')
        url = get_current_url(environ)
        logging.debug("Profiling call for '%s %s'", method, url)
        try:
            res = self.run_profile(self.app, (environ, start_response))
        except Exception, e:
            logging.exception("Exception while profiling '%s %s'", method, url)
            raise
        return res

    __call__ = profile

    def run_profile(self, app, *args, **kwargs):
        """ Override in subclasses.

        Several profilers available for python use the same call signature,
        therefore simplifying the implementation.
        """
        raise NotImplementedError()

    def shutdown(self):
        """ Override in subclasses to clean up when server/script shuts down. """
        pass

class CProfileMiddleware(ProfilerMiddleware):
    """ A profiler based on the the cProfile module from the standard lib. """
    def __init__(self, app, filename):
        super(CProfileMiddleware, self).__init__(app)
        import cProfile
        self._profile = cProfile.Profile()
        self._filename = filename
        self.run_profile = self._profile.runcall

    def shutdown(self):
        self._profile.dump_stats(self._filename)

class HotshotMiddleware(ProfilerMiddleware):
    """ A profiler based on the more recent hotshot module from the stdlib. """
    def __init__(self, app, *args, **kwargs):
        super(HotshotMiddleware, self).__init__(app)
        import hotshot
        self._profile = hotshot.Profile(*args, **kwargs)
        self.run_profile = self._profile.runcall

    def shutdown(self):
        self._profile.close()

class PycallgraphMiddleware(ProfilerMiddleware):
    """ A call graphing middleware utilizing the pycallgraph 3rd party
    module (available at http://pycallgraph.slowchop.com/). """
    def __init__(self, app, filename):
        super(PycallgraphMiddleware, self).__init__(app)
        import pycallgraph
        pycallgraph.settings['include_stdlib'] = False
        self._filename = filename
        globs = ['pycallgraph.*', 'unknown.*']
        f = pycallgraph.GlobbingFilter(exclude=globs, max_depth=9999)
        self._filter = f
        self.pycallgraph = pycallgraph

    def run_profile(self, app, *args, **kwargs):
        pycallgraph = self.pycallgraph
        pycallgraph.start_trace(reset=True, filter_func=self._filter)
        try:
            return app(*args, **kwargs)
        finally:
            pycallgraph.stop_trace()

    def shutdown(self):
        fname = self._filename
        pycallgraph = self.pycallgraph
        if fname.endswith('.png'):
            logging.info("Saving the rendered callgraph to '%s'", fname)
            pycallgraph.make_dot_graph(fname)
        elif fname.endswith('.dot'):
            logging.info("Saving the raw callgraph to '%s'", fname)
            pycallgraph.save_dot(fname)
