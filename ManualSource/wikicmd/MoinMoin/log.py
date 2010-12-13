# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - init "logging" system

    WARNING
    -------
    logging must be configured VERY early, before the code in log.getLogger
    gets executed. Thus, logging is configured either by:
    a) an environment variable MOINLOGGINGCONF that contains the path/filename
       of a logging configuration file - this method overrides all following
       methods (except if it can't read or use that configuration, then it
       will use c))
    b) by an explicit call to MoinMoin.log.load_config('logging.conf') -
       you need to do this very early or a) or c) will happen before
    c) by using a builtin fallback logging conf

    If logging is not yet configured, log.getLogger will do an implicit
    configuration call - then a) or c) is done.

    Usage (for wiki server admins)
    ------------------------------
    Either use something like this in some shell script:
    MOINLOGGINGCONF=/path/to/logging.conf
    export MOINLOGGINGCONF

    Or, modify your server adaptor script (e.g. moin.cgi) to do this:

    from MoinMoin import log
    log.load_config('wiki/config/logging/logfile') # XXX please fix this path!

    You have to fix that path to use a logging configuration matching your
    needs (we provide some examples in the path given there, it is relative to
    the uncompressed moin distribution archive - if you use some moin package,
    you maybe find it under /usr/share/moin/).
    It is likely that you also have to edit the sample logging configurations
    we provide (e.g. to fix the logfile location).

    Usage (for developers)
    ----------------------
    If you write code for moin, do this at top of your module:

    from MoinMoin import log
    logging = log.getLogger(__name__)

    This will create a logger with 'MoinMoin.your.module' as name.
    The logger can optionally get configured in the logging configuration.
    If you don't configure it, some upperlevel logger (e.g. the root logger)
    will do the logging.

    @copyright: 2008 MoinMoin:ThomasWaldmann,
                2007 MoinMoin:JohannesBerg
    @license: GNU GPL, see COPYING for details.
"""

# This is the "last resort" fallback logging configuration for the case
# that load_config() is either not called at all or with a non-working
# logging configuration.
# See http://www.python.org/doc/lib/logging-config-fileformat.html
# We just use stderr output by default, if you want anything else,
# you will have to configure logging.
logging_defaults = {
    'loglevel': 'INFO',
}
logging_config = """\
[loggers]
keys=root

[handlers]
keys=stderr

[formatters]
keys=default

[logger_root]
level=%(loglevel)s
handlers=stderr

[handler_stderr]
class=StreamHandler
level=NOTSET
formatter=default
args=(sys.stderr, )

[formatter_default]
format=%(asctime)s %(levelname)s %(name)s:%(lineno)d %(message)s
datefmt=
class=logging.Formatter
"""

import os
import logging
import logging.config
import logging.handlers  # needed for handlers defined there being configurable in logging.conf file

configured = False
fallback_config = False

import warnings

# 'CacheNeedsUpdate' string exception in Page.py is supported for backwards compat reasons:
warnings.filterwarnings('ignore', r'catching of string exceptions is deprecated', module='MoinMoin.Page')

# TODO: subprocess was added in python 2.4, we now can refactor the code to use it and remove this:
warnings.filterwarnings('ignore', r'The popen\d? module is deprecated.  Use the subprocess module.')


def _log_warning(message, category, filename, lineno, file=None, line=None):
    # for warnings, we just want to use the logging system, not stderr or other files
    msg = "%s:%s: %s: %s" % (filename, lineno, category.__name__, message)
    logger = getLogger(__name__)
    logger.warning(msg) # Note: the warning will look like coming from here,
                        # but msg contains info about where it really comes from


def load_config(conf_fname=None):
    """ load logging config from conffile """
    global configured
    err_msg = None
    conf_fname = os.environ.get('MOINLOGGINGCONF', conf_fname)
    if conf_fname:
        try:
            conf_fname = os.path.abspath(conf_fname)
            logging.config.fileConfig(conf_fname)
            configured = True
            l = getLogger(__name__)
            l.info('using logging configuration read from "%s"' % conf_fname)
            warnings.showwarning = _log_warning
        except Exception, err: # XXX be more precise
            err_msg = str(err)
    if not configured:
        # load builtin fallback logging config
        from StringIO import StringIO
        config_file = StringIO(logging_config)
        logging.config.fileConfig(config_file, logging_defaults)
        configured = True
        l = getLogger(__name__)
        if err_msg:
            l.warning('load_config for "%s" failed with "%s".' % (conf_fname, err_msg))
        warnings.showwarning = _log_warning


def getLogger(name):
    """ wrapper around logging.getLogger, so we can do some more stuff:
        - preprocess logger name
        - patch loglevel constants into logger object, so it can be used
          instead of the logging module
    """
    if not configured:
        load_config()
    logger = logging.getLogger(name)
    for levelnumber, levelname in logging._levelNames.items():
        if isinstance(levelnumber, int): # that list has also the reverse mapping...
            setattr(logger, levelname, levelnumber)
    return logger

