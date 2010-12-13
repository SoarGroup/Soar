"""
Daemon - daemon script and controller

This module is based on twisted.scripts.twistd, modified by Nir Soffer
to work with non Twisted code.

The Daemon class, represents a background process using a pid
file. When you create an instance, the process may be running or not.
After creating an instance, you can call one of its do_xxx() methods.

The DaemonScript class is a script that control a daemon, with a
functionality similar to apachectl. To create a daemon script, create an
instacne and call its run() method.

Typical usage::

    # Daemon script
    import daemon
    import myserver
    script = daemon.DaemonScript('myserver', 'myserver.pid',
                                 myserver.run, myserver.Config)
    script.run()


Copyright (c) 2005 Nir Soffer <nirs@freeshell.org>

Twisted, the Framework of Your Internet
Copyright (c) 2001-2004 Twisted Matrix Laboratories.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

import sys, os, errno, signal, time


class Error(Exception):
    """ Daemon error """


class Daemon:
    """ A background process

    Represent a background process, which may be running or not. The
    process can be started, stopped, restarted or killed.
    """
    commandPrefix = 'do_'

    def __init__(self, name, pidfile, function, *args, **kw):
        """ Create a daemon

        @param name: name of the process
        @param pidfile: pid filename
        @param function: the server main function, will block until the
            server is done.
        @param args: arguments to pass to function
        @param kw: keyword arguments to pass to function
        """
        self.name = name
        self.function = function
        self.args = args
        self.kw = kw
        self.pidFile = os.path.abspath(pidfile)

    # --------------------------------------------------------------------
    # Commands

    def do_start(self):
        """ Start the daemon process

        Start will daemonize then block until the server is killed and
        then cleanup the pid file on the way out.
        """
        running, pid = self.status()
        if running:
            raise Error("another application is running with pid %s "
                        "(try restart)" % pid)
        self.daemonize()
        self.writePID()
        try:
            self.function(*self.args, **self.kw)
        finally:
            self.removePID()

    def do_stop(self):
        """ Stop the daemon process

        Terminate or raise an error we can't handle here. On success,
        the pid file will be cleaned by the terminated process.
        """
        running, pid = self.status()
        if not running:
            return self.log("%s is not running" % self.name)
        os.kill(pid, signal.SIGINT)

    def do_kill(self):
        """ Kill the daemon process

        Kill or raise an error which we can't handle here. Clean the
        pid file for the killed process.
        """
        running, pid = self.status()
        if not running:
            return self.log("%s is not running" % self.name)
        os.kill(pid, signal.SIGKILL)
        self.removePID()

    def do_restart(self):
        """ stop, wait until pid file gone and start again """
        running, pid = self.status()
        if not running:
            self.log("%s is not running, trying to start" % self.name)
        else:
            self.do_stop()
        timeoutSeconds = 2.0
        start = time.time()
        while time.time() - start < timeoutSeconds:
            running, pid = self.status()
            if not running:
                break
            time.sleep(0.1)
        else:
            raise Error("could not start after %s seconds" % timeoutSeconds)
        self.do_start()

    # -------------------------------------------------------------------
    # Private

    def status(self):
        """ Return status tuple (running, pid)

        See http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC18
        """
        running = False
        pid = self.readPID()
        if pid is not None:
            try:
                os.kill(pid, 0)
                running = True
            except OSError, err:
                if err.errno == errno.ESRCH:
                    # No such process or security enhancements are causing
                    # the system to deny its existence.
                    self.log("removing stale pid file: %s" % self.pidFile)
                    self.removePID()
                else:
                    raise
        return running, pid

    def readPID(self):
        """ Return the pid from the pid file

        If there is no pid file, return None. If pid file is corrupted,
        remove it. If its not readable, raise.
        """
        pid = None
        try:
            pid = int(file(self.pidFile).read())
        except IOError, err:
            if err.errno != errno.ENOENT:
                raise
        except ValueError:
            self.warn("removing corrupted pid file: %s" % self.pidFile)
            self.removePID()
        return pid

    def daemonize(self):
        """ Make the current process a daemon

        See http://www.erlenstar.demon.co.uk/unix/faq_toc.html#TOC16
        """
        if os.fork():   # launch child and...
            os._exit(0) # kill off parent
        os.setsid()
        if os.fork():   # launch child and...
            os._exit(0) # kill off parent again.
        os.umask(0077)
        null = os.open('/dev/null', os.O_RDWR)
        for i in range(3):
            try:
                os.dup2(null, i)
            except OSError, e:
                if e.errno != errno.EBADF:
                    raise
        os.close(null)

    def writePID(self):
        pid = str(os.getpid())
        open(self.pidFile, 'wb').write(pid)

    def removePID(self):
        try:
            os.remove(self.pidFile)
        except OSError, err:
            if err.errno != errno.ENOENT:
                raise

    def warn(self, message):
        self.log('warning: %s' % message)

    def log(self, message):
        """ TODO: does it work after daemonize? """
        sys.stderr.write(message + '\n')


class DaemonScript(Daemon):
    """ A script controlling a daemon

    TODO: add --pid-dir option?
    """

    def run(self):
        """ Check commandline arguments and run a command """
        args = len(sys.argv)
        if args == 1:
            self.usage('nothing to do')
        elif args > 2:
            self.usage("too many arguments")
        try:
            command = sys.argv[1]
            func = getattr(self, self.commandPrefix + command)
            func()
        except AttributeError:
            self.usage('unknown command %r' % command)
        except Exception, why:
            sys.exit("error: %s" % str(why))

    def usage(self, message):
        sys.stderr.write('error: %s\n' % message)
        sys.stderr.write(self.help())
        sys.exit(1)

    def help(self):
        return """
%(name)s - MoinMoin daemon

usage: %(name)s command

commands:
  start     start the server
  stop      stop the server
  restart   stop then start the server
  kill      kill the server

@copyright: 2004-2005 Thomas Waldmann, Nir Soffer
@license: GNU GPL, see COPYING for details.
""" % {
    'name': self.name,
}

