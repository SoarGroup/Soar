# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Flup based WSGI adapters

    This module provides adapters between popular gateway interfaces
    like CGI, FastCGI, SCGI and AJP to the MoinMoin WSGI application.
    They are based on the adapters in the flup package and upon the
    MoinMoin.frontend.ServerFrontEnd to provide configuration via
    command line switches.

    Typically they are simply run from the CGI-scripts like this:

    > from MoinMoin.web.flup_frontend import CGIFrontEnd
    > CGIFrontEnd().run()

    They automatically parse the options given on the commandline and
    behave accordingly. Flup makes it possible to serve FCGI, SCGI and
    AJP from a bound network or unix socket, in different flavours of
    multiprocessing/multithreading.

    @copyright: 2008 MoinMoin:FlorianKrupicka,
                2009 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

import os, sys

try:
    import flup.server.fcgi
    have_flup = True
    try:
        import flup.server.fcgi_single
        have_singlepatch = True
    except ImportError:
        have_singlepatch = False
except ImportError:
    have_flup = False

from MoinMoin.web.frontend import ServerFrontEnd, FrontEnd, FrontEndNotAvailable

from MoinMoin import log
logging = log.getLogger(__name__)

if have_flup:
    class FlupFrontEnd(ServerFrontEnd):
        def add_options(self):
            super(FlupFrontEnd, self).add_options()
            parser = self.parser
            parser.add_option("--min-spare", dest="min_spare", type="int", metavar='MIN',
                              help=("Minimum spare threads/processes (when "
                                    "using threaded or forking servers)."))
            parser.add_option("--max-spare", dest="max_spare", type="int", metavar='MAX',
                              help=("Maximum spare threads/processes (when "
                                    "using threaded or forking servers)."))
            parser.add_option("--max-childs", dest="max_childs", type="int", metavar='CHILDS',
                              help=("Hard upper limit on threads/processes "
                                    "(when using threaded or forking servers)."))
            parser.add_option("-t", "--type", dest="server_type", metavar='TYPE',
                              help=("Type of server to use, e.g. single/threaded"
                                    "/forking. Defaults to 'single' when not "
                                    "bound to a socket and to 'threaded' when it is"))

        def run_server(self, application, options):
            server_type = options.server_type

            if not server_type:
                if 'single' in self.server_types:
                    server_type = (options.port and 'threaded') or 'single'
                else:
                    server_type = 'threaded'

            if server_type not in self.server_types:
                raise TypeError("Unknown server type '%s'" % options.server_type)

            multi = server_type in ('threaded', 'forking')

            mod = self.server_types[server_type]
            mod = __import__(mod, globals(), locals(), ['WSGIServer'])
            WSGIServerWrapped = mod.WSGIServer

            class WSGIServer(WSGIServerWrapped):
                # note: base class uses debug=False as default. as we use string values,
                # better explicitely pass in "off", "web" or "external".
                def error(self, req):
                    """ Override the default handler, so it implements debug=web/external/off. """
                    if self.debug == 'external':
                        raise
                    elif self.debug == 'web':
                        import cgitb
                        req.stdout.write('Content-Type: text/html\r\n\r\n' +
                                         cgitb.html(sys.exc_info()))
                    else: # 'off'
                        errorpage = """<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html><head>
<title>Unhandled Exception</title>
</head><body>
<h1>Unhandled Exception</h1>
<p>An unhandled exception was thrown by the application.</p>
</body></html>
"""
                        req.stdout.write('Content-Type: text/html\r\n\r\n' + errorpage)


            kwargs = {}

            kwargs['debug'] = options.debug or os.environ.get('MOIN_DEBUGGER', 'off')

            if options.port:
                kwargs['bindAddress'] = (options.interface, options.port)
            elif options.interface and (
                 options.interface.startswith('/') or options.interface.startswith('./')):
                kwargs['bindAddress'] = options.interface

            if options.min_spare and multi:
                kwargs['minSpare'] = options.min_spare
            if options.max_spare and multi:
                kwargs['maxSpare'] = options.max_spare
            if options.max_childs and multi:
                if server_type == 'threaded':
                    kwargs['maxThreads'] = options.max_childs
                else:
                    kwargs['maxChildren'] = options.max_childs
            logging.debug("WSGIServer(%r, %r)" % (application, kwargs))
            return WSGIServer(application, **kwargs).run()

    class CGIFrontEnd(FlupFrontEnd):
        server_types = {'threaded': 'flup.server.fcgi',
                        'forking': 'flup.server.fcgi_fork'}
        if have_singlepatch:
            server_types['single'] = 'flup.server.fcgi_single'

        def run(self, args=None):
            if 'GATEWAY_INTERFACE' in os.environ:
                sys.argv = []
            super(CGIFrontEnd, self).run(args)

    class SCGIFrontEnd(FlupFrontEnd):
        server_types = {'threaded': 'flup.server.scgi',
                        'forking': 'flup.server.scgi_fork'}

    class AJPFrontEnd(FlupFrontEnd):
        server_types = {'threaded': 'flup.server.ajp',
                        'forking': 'flup.server.ajp_fork'}
else:
    class CGIFrontEnd(FrontEnd):
        """ Simple WSGI CGI Adapter for fallback if flup is not installed. """
        def __init__(self):
            logging.warning("No flup-package installed, only basic CGI "
                            "support is available.")
            super(CGIFrontEnd, self).__init__()

        def run(self, args=None):
            if 'GATEWAY_INTERFACE' in os.environ:
                sys.argv = []
            super(CGIFrontEnd, self).run(args)

        def run_server(self, application, options):
            from MoinMoin.web._fallback_cgi import WSGIServer
            return WSGIServer(application).run()

    _ERROR = """
The flup package is not installed on your system. To make use of FCGI,
SCGI or AJP adapters, you have to install it first. The MoinMoin source
distribution provides a flup package in the contrib/flup-server
directory. It is also patched to support non-threaded & non-forking
behaviour. See contrib/flup-server/NOTES.moin for more information.
"""
    def SCGIFrontEnd():
        raise FrontEndNotAvailable(_ERROR)
    def AJPFrontEnd():
        raise FrontEndNotAvailable(_ERROR)
