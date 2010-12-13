# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Extension Script Package

    @copyright: 2000-2002 Juergen Hermann <jh@web.de>,
                2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

import os, sys, time
from StringIO import StringIO

flag_quiet = 0

# ScriptRequest -----------------------------------------------------------

class ScriptRequest(object):
    """this is for scripts (MoinMoin/script/*) running from the commandline (CLI)
       or from the xmlrpc server (triggered by a remote xmlrpc client).

       Every script needs to do IO using this ScriptRequest class object -
       IT IS DIFFERENT from the usual "request" you have in moin (easily to be seen
       when you look at an xmlrpc script invocation: request.write will write to the
       xmlrpc "channel", but scriptrequest.write needs to write to some buffer we
       transmit later as an xmlrpc function return value.
    """
    def __init__(self, instream, outstream, errstream):
        self.instream = instream
        self.outstream = outstream
        self.errstream = errstream

    def read(self, n=None):
        if n is None:
            data = self.instream.read()
        else:
            data = self.instream.read(n)
        return data

    def write(self, data):
        self.outstream.write(data)

    def write_err(self, data):
        self.errstream.write(data)


class ScriptRequestCLI(ScriptRequest):
    """ When a script runs directly on the shell, we just use the CLI request
        object (see MoinMoin.request.request_cli) to do I/O (which will use stdin/out/err).
    """
    def __init__(self, request):
        self.request = request

    def read(self, n=None):
        return self.request.read(n)

    def write(self, data):
        return self.request.write(data)

    def write_err(self, data):
        return self.request.write(data) # XXX use correct request method - log, error, whatever.

class ScriptRequestStrings(ScriptRequest):
    """ When a script gets run by our xmlrpc server, we have the input as a
        string and we also need to catch the output / error output as strings.
    """
    def __init__(self, instr):
        self.instream = StringIO(instr)
        self.outstream = StringIO()
        self.errstream = StringIO()

    def fetch_output(self):
        outstr = self.outstream.get_value()
        errstr = self.errstream.get_value()
        self.outstream.close()
        self.errstream.close()
        return outstr, errstr


# Logging -----------------------------------------------------------------

def fatal(msgtext, **kw):
    """ Print error msg to stderr and exit. """
    sys.stderr.write("\n\nFATAL ERROR: " + msgtext + "\n")
    sys.exit(1)


def log(msgtext):
    """ Optionally print error msg to stderr. """
    if not flag_quiet:
        sys.stderr.write(msgtext + "\n")


# Commandline Support --------------------------------------------------------

class Script:
    def __init__(self, cmd, usage, argv=None, def_values=None):
        #print "argv:", argv, "def_values:", repr(def_values)
        if argv is None:
            self.argv = sys.argv[1:]
        else:
            self.argv = argv
        self.def_values = def_values

        global _start_time
        _start_time = time.clock()

        import optparse
        from MoinMoin import version

        rev = "%s %s [%s]" % (version.project, version.release, version.revision)
        sys.argv[0] = cmd

        self.parser = optparse.OptionParser(
            usage="%(cmd)s [command] %(usage)s" % {'cmd': os.path.basename(sys.argv[0]), 'usage': usage, },
            version=rev, add_help_option=False)
        self.parser.allow_interspersed_args = False
        if def_values:
            self.parser.set_defaults(**def_values.__dict__)
        self.parser.add_option(
            "-q", "--quiet",
            action="store_true", dest="quiet",
            help="Be quiet (no informational messages)"
        )
        self.parser.add_option(
            "--show-timing",
            action="store_true", dest="show_timing", default=False,
            help="Show timing values [default: False]"
        )

    def run(self, showtime=1):
        """ Run the main function of a command. """
        global flag_quiet
        try:
            try:
                self.options, self.args = self.parser.parse_args(self.argv)
                flag_quiet = self.options.quiet
                # ToDo check if we need to initialize request (self.init_request())
                self.mainloop()
            except KeyboardInterrupt:
                log("*** Interrupted by user!")
            except SystemExit:
                showtime = 0
                raise
        finally:
            if showtime:
                self.logRuntime()

    def logRuntime(self):
        """ Print the total command run time. """
        if self.options.show_timing:
            log("Needed %.3f secs." % (time.clock() - _start_time, ))


class MoinScript(Script):
    """ Moin main script class """

    def __init__(self, argv=None, def_values=None):
        Script.__init__(self, "moin", "[general options] command subcommand [specific options]", argv, def_values)
        # those are options potentially useful for all sub-commands:
        self.parser.add_option(
            "--config-dir", metavar="DIR", dest="config_dir",
            help=("Path to the directory containing the wiki "
                  "configuration files. [default: current directory]")
        )
        self.parser.add_option(
            "--wiki-url", metavar="WIKIURL", dest="wiki_url",
            help="URL of a single wiki to migrate e.g. http://localhost/mywiki/ [default: CLI]"
        )
        self.parser.add_option(
            "--page", dest="page", default='',
            help="wiki page name [default: all pages]"
        )

    def _update_option_help(self, opt_string, help_msg):
        """ Update the help string of an option. """
        for option in self.parser.option_list:
            if option.get_opt_string() == opt_string:
                option.help = help_msg
                break

    def init_request(self):
        """ create request """
        from MoinMoin.web.contexts import ScriptContext
        url = self.options.wiki_url or None
        self.request = ScriptContext(url, self.options.page)

    def mainloop(self):
        # Insert config dir or the current directory to the start of the path.
        config_dir = self.options.config_dir
        if config_dir:
            if os.path.isdir(config_dir):
                sys.path.insert(0, os.path.abspath(config_dir))
            else:
                fatal("bad path given to --config-dir option")

        args = self.args
        if len(args) < 2:
            self.parser.print_help()
            fatal("""You must specify a command module and name:

moin ... account check ...
moin ... account create ...
moin ... account disable ...
moin ... account resetpw ...

moin ... cli show ...

moin ... export dump ...

moin ... import irclog ...
moin ... import wikipage ...

moin ... index build ...

moin ... maint cleancache ...
moin ... maint cleanpage ...
moin ... maint globaledit ...
moin ... maint makecache ...
moin ... maint mkpagepacks ...
moin ... maint reducewiki ...

moin ... migration data ...

moin ... server standalone ...

moin ... xmlrpc mailimport ...
moin ... xmlrpc remote ...

General options:
    Most commands need some general parameters before command subcommand:
    --config-dir=/config/directory
        Mandatory for most commands and specifies the directory that contains
        your wikiconfig.py (or farmconfig.py).

    --wiki-url=http://wiki.example.org/
        Mandatory for most commands and specifies the url of the wiki you like
        to operate on.

Specific options:
    Most commands need additional parameters after command subcommand.

    To obtain additonal help on a command use 'moin module subcommand --help'
""")

        cmd_module, cmd_name = args[:2]
        from MoinMoin import wikiutil
        try:
            plugin_class = wikiutil.importBuiltinPlugin('script.%s' % cmd_module, cmd_name, 'PluginScript')
        except wikiutil.PluginMissingError:
            fatal("Command plugin %r, command %r was not found." % (cmd_module, cmd_name))

        # We have to use the args list here instead of optparse, as optparse only
        # deals with things coming before command subcommand.
        if "--help" in args or "-h" in args:
            print "MoinMoin Help - %s/ %s\n" % (cmd_module, cmd_name)
            print plugin_class.__doc__
            print "Command line reference:"
            print "======================="
            plugin_class(args[2:], self.options).parser.print_help()
        else:
            plugin_class(args[2:], self.options).run() # all starts again there

