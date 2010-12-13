# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - New slimmed down WSGI Request.

    @copyright: 2008-2008 MoinMoin:FlorianKrupicka
    @license: GNU GPL, see COPYING for details.
"""

import re
from StringIO import StringIO

from werkzeug import Request as RequestBase
from werkzeug import BaseResponse, ETagResponseMixin, \
                     CommonResponseDescriptorsMixin, WWWAuthenticateMixin
from werkzeug.wrappers import ResponseStream
from werkzeug import EnvironHeaders, Headers, HeaderSet
from werkzeug import Href, create_environ, url_encode, cached_property
from werkzeug import Client # used by tests

from MoinMoin import config

from MoinMoin import log
logging = log.getLogger(__name__)

class MoinMoinFinish(Exception):
    """ Raised to jump directly to end of run() function, where finish is called """

class ModifiedResponseStreamMixin(object):
    """
    to avoid .stream attributes name collision when we mix together Request
    and Response, we use "out_stream" instead of "stream" in the original
    ResponseStreamMixin
    """
    @cached_property
    def out_stream(self):
        """The response iterable as write-only stream."""
        return ResponseStream(self)

class ResponseBase(BaseResponse, ETagResponseMixin, ModifiedResponseStreamMixin,
                   CommonResponseDescriptorsMixin,
                   WWWAuthenticateMixin):
    """
    similar to werkzeug.Response, but with ModifiedResponseStreamMixin
    """

class Request(ResponseBase, RequestBase):
    """ A full featured Request/Response object.

    To better distinguish incoming and outgoing data/headers,
    incoming versions are prefixed with 'in_' in contrast to
    original Werkzeug implementation.
    """
    charset = config.charset
    encoding_errors = 'replace'
    default_mimetype = 'text/html'
    given_config = None # if None, load wiki config from disk

    # get rid of some inherited descriptors
    headers = None

    def __init__(self, environ, populate_request=True, shallow=False):
        ResponseBase.__init__(self)
        RequestBase.__init__(self, environ, populate_request, shallow)
        self.href = Href(self.script_root or '/', self.charset)
        self.abs_href = Href(self.url_root, self.charset)
        self.headers = Headers([('Content-Type', 'text/html')])
        self.response = []
        self.status_code = 200

    # Note: we inherit a .stream attribute from RequestBase and this needs
    # to refer to the input stream because inherited functionality of werkzeug
    # base classes will access it as .stream.
    # The output stream is .out_stream (see above).
    # TODO keep request and response separate, don't mix them together

    @cached_property
    def in_headers(self):
        return EnvironHeaders(self.environ)


class TestRequest(Request):
    """ Request with customized `environ` for test purposes. """
    def __init__(self, path="/", query_string=None, method='GET',
                 content_type=None, content_length=0, form_data=None,
                 environ_overrides=None):
        """
        For parameter reference see the documentation of the werkzeug
        package, especially the functions `url_encode` and `create_environ`.
        """
        input_stream = None

        if form_data is not None:
            form_data = url_encode(form_data)
            content_type = 'application/x-www-form-urlencoded'
            content_length = len(form_data)
            input_stream = StringIO(form_data)
        environ = create_environ(path=path, query_string=query_string,
                                 method=method, input_stream=input_stream,
                                 content_type=content_type,
                                 content_length=content_length)

        environ['HTTP_USER_AGENT'] = 'MoinMoin/TestRequest'
        # must have reverse lookup or tests will be extremely slow:
        environ['REMOTE_ADDR'] = '127.0.0.1'

        if environ_overrides:
            environ.update(environ_overrides)

        super(TestRequest, self).__init__(environ)

def evaluate_request(request):
    """ Evaluate a request and returns a tuple of application iterator,
    status code and list of headers. This method is meant for testing
    purposes.
    """
    output = []
    headers_set = []
    def start_response(status, headers, exc_info=None):
        headers_set[:] = [status, headers]
        return output.append
    result = request(request.environ, start_response)

    # any output via (WSGI-deprecated) write-callable?
    if output:
        result = output
    return (result, headers_set[0], headers_set[1])

