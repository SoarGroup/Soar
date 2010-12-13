# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - HTTP exceptions

    Customization of werkzeug.exceptions classes for use in MoinMoin.

    @copyright: 2008-2008 MoinMoin:FlorianKrupicka
    @license: GNU GPL, see COPYING for details.
"""

from werkzeug import exceptions

HTTPException = exceptions.HTTPException

class SurgeProtection(exceptions.ServiceUnavailable):
    """ A surge protection error in MoinMoin is based on the HTTP status
    `Service Unavailable`. This HTTP exception gives a short description
    on what triggered the surge protection mechanism to the user.
    """

    name = 'Surge protection'
    description = (
        "<strong>Warning:</strong>"
        "<p>You triggered the wiki's surge protection by doing too many requests in a short time.</p>"
        "<p>Please make a short break reading the stuff you already got.</p>"
        "<p>When you restart doing requests AFTER that, slow down or you might get locked out for a longer time!</p>"
    )

    def __init__(self, description=None, retry_after=3600):
        exceptions.ServiceUnavailable.__init__(self, description)
        self.retry_after = retry_after

    def get_headers(self, environ):
        headers = exceptions.ServiceUnavailable.get_headers(self, environ)
        headers.append(('Retry-After', '%d' % self.retry_after))
        return headers

class Forbidden(exceptions.Forbidden):
    """
    Override the default description of werkzeug.exceptions.Forbidden to a
    less technical sounding one.
    """
    description = "<p>You are not allowed to access this!</p>"

# handy exception raising
abort = exceptions.abort
