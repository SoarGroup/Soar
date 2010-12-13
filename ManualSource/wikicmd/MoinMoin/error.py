# -*- coding: iso-8859-1 -*-
""" MoinMoin errors

    Supply Error class and sub classes used to raise various errors

    @copyright: 2004-2005 Nir Soffer <nirs@freeshell.org>
    @license: GNU GPL, see COPYING for details.
"""

import sys

from MoinMoin import config

class Error(Exception):
    """ Base class for moin moin errors

    Use this class when you raise errors or create sub classes that
    may be used to display non ASCII error message.

    Standard errors work safely only with strings using ascii or
    unicode. This class can be used safely with both strings using
    config.charset and unicode.

    You can init this class with either unicode or string using
    config.charset encoding. On output, the class will convert the string
    to unicode or the unicode to string, using config.charset.

    When you want to render an error, use unicode() or str() as needed.
    """

    def __init__(self, message):
        """ Initialize an error, decode if needed

        @param message: unicode, str or object that support __unicode__
            and __str__. __str__ should use config.charset.
        """
        self.message = message

    def __unicode__(self):
        """ Return unicode error message """
        if isinstance(self.message, str):
            return unicode(self.message, config.charset)
        else:
            return unicode(self.message)

    def __str__(self):
        """ Return encoded message """
        if isinstance(self.message, unicode):
            return self.message.encode(config.charset)
        else:
            return str(self.message)

    def __getitem__(self, item):
        """ Make it possible to access attributes like a dict """
        return getattr(self, item)


class CompositeError(Error):
    """ Base class for exceptions containing an exception

    Do not use this class but its more specific sub classes.

    Useful for hiding low level error inside high level user error,
    while keeping the inner error information for debugging.

    Example::

        class InternalError(CompositeError):
            ''' Raise for internal errors '''

        try:
            # code that might fail...
        except HairyLowLevelError:
            raise InternalError("Sorry, internal error occurred")

    When showing a traceback, both InternalError traceback and
    HairyLowLevelError traceback are available.
    """

    def __init__(self, message):
        """ Save system exception info before this exception is raised """
        Error.__init__(self, message)
        self.innerException = sys.exc_info()

    def exceptions(self):
        """ Return a list of all inner exceptions """
        all = [self.innerException]
        while 1:
            lastException = all[-1][1]
            try:
                all.append(lastException.innerException)
            except AttributeError:
                break
        return all

class FatalError(CompositeError):
    """ Base class for fatal error we can't handle

    Do not use this class but its more specific sub classes.
    """

class ConfigurationError(FatalError):
    """ Raise when fatal misconfiguration is found """

class InternalError(FatalError):
    """ Raise when internal fatal error is found """

class NoConfigMatchedError(Exception):
    """ we didn't find a configuration for this URL """
    pass

class ConvertError(FatalError):
    """ Raise when html to storage format (e.g. 'wiki') conversion fails """
    name = "MoinMoin Convert Error"


