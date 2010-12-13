# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Common code for frontends (CGI/FCGI/SCGI)

    @copyright: 2008 MoinMoin:FlorianKrupicka
    @license: GNU GPL, see COPYING for details.
"""
import optparse

from MoinMoin.web.serving import make_application

from MoinMoin import log
logging = log.getLogger(__name__)

class FrontEnd(object):
    def __init__(self):
        self.parser = optparse.OptionParser()
        self.add_options()

    def add_options(self):
        parser = self.parser
        parser.add_option("-d", "--debug", dest="debug",
                          help="Debug mode of server (off/web/external, default is to use MOIN_DEBUGGER env var)")
        parser.add_option("-c", "--config-dir", dest="config_dir", metavar="DIR",
                          help=("Path to the directory containing the wiki "
                                "configuration files. Default: current directory"))
        parser.add_option("--htdocs", dest="htdocs",
                          help=("Path to the directory containing Moin's "
                                "static files. Default: use builtin MoinMoin/web/static/htdocs"))
        parser.set_default('htdocs', True)

    def run(self, args=None):
        options, args = self.parser.parse_args(args)
        logging.debug('Options: %r', options)

        application = make_application(shared=options.htdocs)

        try:
            self.run_server(application, options)
        except SystemExit, err:
            # the flup CGIRequest uses sys.exit(0) to terminate
            if err.code: # log a non-zero exit status (0 means no error)
                logging.exception('A SystemExit(%d) exception occurred.' % err.code)
            raise # exit now with this exit status
        except:
            logging.exception('An exception occurred while running %s' % self.__class__.__name__)

class ServerFrontEnd(FrontEnd):
    def add_options(self):
        super(ServerFrontEnd, self).add_options()
        parser = self.parser
        parser.add_option("-p", "--port", dest="port", type="int",
                          help="Set the port to listen on.")
        parser.add_option("-i", "--interface", dest="interface",
                          help=("Set the interface/socket to listen on. If starts "
                                "with '/' or './' it is interpreted as a path "
                                "to a unix socket."))
        # Note: interface default MUST be None, do not set it to something else!
        # Otherwise CGI (and also when the FCGI process is spawned by the web server) won't work.

class FrontEndNotAvailable(Exception):
    """ Raised if a frontend is not available for one reason or another. """
    pass
