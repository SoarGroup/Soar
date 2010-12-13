# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - This module contains additional code related to serving
               requests with the standalone server. It uses werkzeug's
               BaseRequestHandler and overrides some functions that
               need to be handled different in MoinMoin than in werkzeug

    @copyright: 2008-2008 MoinMoin:FlorianKrupicka
    @license: GNU GPL, see COPYING for details.
"""
import os
from MoinMoin import config

from MoinMoin import version, log
logging = log.getLogger(__name__)

# make werkzeug use our logging framework and configuration:
import werkzeug._internal
werkzeug._internal._logger = log.getLogger('werkzeug')

from werkzeug import run_simple
from werkzeug.serving import BaseRequestHandler

class RequestHandler(BaseRequestHandler):
    """
    A request-handler for WSGI, that overrides the default logging
    mechanisms to log via MoinMoin's logging framework.
    """
    server_version = "MoinMoin %s %s" % (version.release,
                                         version.revision)

    # override the logging functions
    def log_request(self, code='-', size='-'):
        self.log_message('"%s" %s %s',
                         self.requestline, code, size)

    def log_error(self, format, *args):
        self.log_message(format, *args)

    def log_message(self, format, *args):
        logging.info("%s %s", self.address_string(), (format % args))

class ProxyTrust(object):
    """
    Middleware that rewrites the remote address according to trusted
    proxies in the forward chain.
    """

    def __init__(self, app, proxies):
        self.app = app
        self.proxies = proxies

    def __call__(self, environ, start_response):
        if 'HTTP_X_FORWARDED_FOR' in environ:
            addrs = environ.pop('HTTP_X_FORWARDED_FOR').split(',')
            addrs = [addr.strip() for addr in addrs]
        elif 'REMOTE_ADDR' in environ:
            addrs = [environ['REMOTE_ADDR']]
        else:
            addrs = [None]
        result = [addr for addr in addrs if addr not in self.proxies]
        if result:
            environ['REMOTE_ADDR'] = result[-1]
        elif addrs[-1] is not None:
            environ['REMOTE_ADDR'] = addrs[-1]
        else:
            del environ['REMOTE_ADDR']
        return self.app(environ, start_response)

def make_application(shared=None, trusted_proxies=None):
    """
    Make an instance of the MoinMoin WSGI application. This involves
    wrapping it in middlewares as needed (static files, debugging, etc.).

    @param shared: see MoinMoin.web.static.make_static_serving_app.
                   If falsy, do not use static serving app.
    @param trusted_proxies: list of trusted proxies. If None or empty, do not
                            use the ProxyTrust middleware.
    @rtype: callable
    @return: a WSGI callable
    """
    from MoinMoin.wsgiapp import application

    if trusted_proxies:
        application = ProxyTrust(application, trusted_proxies)

    if shared:
        from MoinMoin.web.static import make_static_serving_app
        application = make_static_serving_app(application, shared)

    return application

def switch_user(uid, gid=None):
    """ Switch identity to safe user and group

    Does not support Windows, because the necessary calls are not available.
    TODO: can we use win32api calls to achieve the same effect on Windows?

    Raise RuntimeError if can't switch or trying to switch to root.
    """
    # no switch on windows
    if os.name == 'nt':
        return

    import pwd, grp
    if isinstance(uid, basestring):
        try:
            uid = pwd.getpwnam(uid)[2]
        except KeyError:
            raise RuntimeError("Unknown user: '%s', check user setting" % uid)
    if gid is not None and isinstance(gid, basestring):
        try:
            gid = grp.getgrnam(gid)[2]
        except KeyError:
            raise RuntimeError("Unknown group: '%s', check group setting" % gid)

    if uid == 0 or gid == 0:
        # We will not run as root. If you like to run a web
        # server as root, then hack this code.
        raise RuntimeError('will not run as root!')
    try:
        if gid:
            os.setgid(gid)
        os.setuid(uid)
    except (OSError, AttributeError):
        # Either we can't switch, or we are on windows, which does not have
        # those calls.
        raise RuntimeError("can't change uid/gid to %s/%s" % (uid, gid))
    logging.info("Running as uid/gid %d/%d" % (uid, gid))

def run_server(hostname='localhost', port=8080,
               docs=True,
               debug='off',
               user=None, group=None,
               threaded=True,
               **kw):
    """ Run a standalone server on specified host/port. """
    application = make_application(shared=docs)

    if port < 1024:
        if os.name == 'posix' and os.getuid() != 0:
            raise RuntimeError('Must run as root to serve port number under 1024. '
                               'Run as root or change port setting.')

    if user:
        switch_user(user, group)

    if debug == 'external':
        # no threading is better for debugging, the main (and only)
        # thread then will just terminate when an exception happens
        threaded = False

    run_simple(hostname=hostname, port=port,
               application=application,
               threaded=threaded,
               use_debugger=(debug == 'web'),
               passthrough_errors=(debug == 'external'),
               request_handler=RequestHandler,
               **kw)

