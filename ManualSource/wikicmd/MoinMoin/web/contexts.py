# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Context objects which are passed thru instead of the classic
               request objects. Currently contains legacy wrapper code for
               a single request object.

    @copyright: 2008-2008 MoinMoin:FlorianKrupicka
    @license: GNU GPL, see COPYING for details.
"""

import time, inspect, StringIO, sys, warnings

from werkzeug import Headers, http_date, create_environ, redirect, abort
from werkzeug.exceptions import Unauthorized, NotFound

from MoinMoin import i18n, error, user, config, wikiutil
from MoinMoin.config import multiconfig
from MoinMoin.formatter import text_html
from MoinMoin.theme import load_theme_fallback
from MoinMoin.util.clock import Clock
from MoinMoin.web.request import Request, MoinMoinFinish
from MoinMoin.web.utils import UniqueIDGenerator
from MoinMoin.web.exceptions import Forbidden, SurgeProtection

from MoinMoin import log
logging = log.getLogger(__name__)
NoDefault = object()

class EnvironProxy(property):
    """ Proxy attribute lookups to keys in the environ. """
    def __init__(self, name, default=NoDefault):
        """
        An entry will be proxied to the supplied name in the .environ
        object of the property holder. A factory can be supplied, for
        values that need to be preinstantiated. If given as first
        parameter name is taken from the callable too.

        @param name: key (or factory for convenience)
        @param default: literal object or callable
        """
        if not isinstance(name, basestring):
            default = name
            name = default.__name__
        self.name = 'moin.' + name
        self.default = default
        property.__init__(self, self.get, self.set, self.delete)

    def get(self, obj):
        if self.name in obj.environ:
            res = obj.environ[self.name]
        else:
            factory = self.default
            if factory is NoDefault:
                raise AttributeError(self.name)
            elif hasattr(factory, '__call__'):
                res = obj.environ.setdefault(self.name, factory(obj))
            else:
                res = obj.environ.setdefault(self.name, factory)
        return res

    def set(self, obj, value):
        obj.environ[self.name] = value

    def delete(self, obj):
        del obj.environ[self.name]

    def __repr__(self):
        return "<%s for '%s'>" % (self.__class__.__name__,
                                  self.name)

class Context(object):
    """ Standard implementation for the context interface.

    This one wraps up a Moin-Request object and the associated
    environ and also keeps track of it's changes.
    """
    def __init__(self, request):
        assert isinstance(request, Request)

        self.request = request
        self.environ = environ = request.environ
        self.personalities = self.environ.setdefault(
            'context.personalities', []
        )
        self.personalities.append(self.__class__.__name__)

    def become(self, cls):
        """ Become another context, based on given class.

        @param cls: class to change to, must be a sister class
        @rtype: boolean
        @return: wether a class change took place
        """
        if self.__class__ is cls:
            return False
        else:
            self.personalities.append(cls)
            self.__class__ = cls
            return True

    def __repr__(self):
        return "<%s %r>" % (self.__class__.__name__, self.personalities)

class BaseContext(Context):
    """ Implements a basic context, that provides some common attributes.
    Most attributes are lazily initialized via descriptors. """

    # first the trivial attributes
    action = EnvironProxy('action', lambda o: o.request.values.get('action', 'show'))
    clock = EnvironProxy('clock', lambda o: Clock())
    user = EnvironProxy('user', lambda o: user.User(o, auth_method='request:invalid'))

    lang = EnvironProxy('lang')
    content_lang = EnvironProxy('content_lang', lambda o: o.cfg.language_default)
    current_lang = EnvironProxy('current_lang')

    html_formatter = EnvironProxy('html_formatter', lambda o: text_html.Formatter(o))
    formatter = EnvironProxy('formatter', lambda o: o.html_formatter)

    page = EnvironProxy('page', None)

    # now the more complex factories
    def cfg(self):
        if self.request.given_config is not None:
            return self.request.given_config('MoinMoin._tests.wikiconfig')
        try:
            self.clock.start('load_multi_cfg')
            cfg = multiconfig.getConfig(self.request.url)
            self.clock.stop('load_multi_cfg')
            return cfg
        except error.NoConfigMatchedError:
            raise NotFound('<p>No wiki configuration matching the URL found!</p>')
    cfg = EnvironProxy(cfg)

    def getText(self):
        lang = self.lang
        def _(text, i18n=i18n, request=self, lang=lang, **kw):
            return i18n.getText(text, request, lang, **kw)
        return _

    getText = property(getText)
    _ = getText

    def isSpiderAgent(self):
        """ Simple check if useragent is a spider bot. """
        cfg = self.cfg
        user_agent = self.http_user_agent
        if user_agent and cfg.cache.ua_spiders:
            return cfg.cache.ua_spiders.search(user_agent) is not None
        return False
    isSpiderAgent = EnvironProxy(isSpiderAgent)

    def rootpage(self):
        from MoinMoin.Page import RootPage
        return RootPage(self)
    rootpage = EnvironProxy(rootpage)

    def rev(self):
        try:
            return int(self.values['rev'])
        except:
            return None
    rev = EnvironProxy(rev)

    def _theme(self):
        self.initTheme()
        return self.theme
    theme = EnvironProxy('theme', _theme)

    # finally some methods to act on those attributes
    def setContentLanguage(self, lang):
        """ Set the content language, used for the content div

        Actions that generate content in the user language, like search,
        should set the content direction to the user language before they
        call send_title!
        """
        self.content_lang = lang
        self.current_lang = lang

    def initTheme(self):
        """ Set theme - forced theme, user theme or wiki default """
        if self.cfg.theme_force:
            theme_name = self.cfg.theme_default
        else:
            theme_name = self.user.theme_name
        load_theme_fallback(self, theme_name)


class HTTPContext(BaseContext):
    """ Context that holds attributes and methods for manipulation of
    incoming and outgoing HTTP data. """

    session = EnvironProxy('session')
    _auth_redirected = EnvironProxy('old._auth_redirected', 0)
    cacheable = EnvironProxy('old.cacheable', 0)
    writestack = EnvironProxy('old.writestack', lambda o: list())

    # proxy some descriptors of the underlying WSGI request, since
    # setting on those does not work over __(g|s)etattr__-proxies
    class _proxy(property):
        def __init__(self, name):
            self.name = name
            property.__init__(self, self.get, self.set, self.delete)
        def get(self, obj):
            return getattr(obj.request, self.name)
        def set(self, obj, value):
            setattr(obj.request, self.name, value)
        def delete(self, obj):
            delattr(obj.request, self.name)

    mimetype = _proxy('mimetype')
    content_type = _proxy('content_type')
    status = _proxy('status')
    status_code = _proxy('status_code')

    del _proxy

    # proxy further attribute lookups to the underlying request first
    def __getattr__(self, name):
        try:
            return getattr(self.request, name)
        except AttributeError, e:
            return super(HTTPContext, self).__getattribute__(name)

    # methods regarding manipulation of HTTP related data
    def read(self, n=None):
        """ Read n bytes (or everything) from input stream. """
        if n is None:
            return self.request.stream.read()
        else:
            return self.request.stream.read(n)

    def makeForbidden(self, resultcode, msg):
        status = {401: Unauthorized,
                  403: Forbidden,
                  404: NotFound,
                  503: SurgeProtection}
        raise status[resultcode](msg)

    def setHttpHeader(self, header):
        logging.warning("Deprecated call to request.setHttpHeader('k:v'), use request.headers.add/set('k', 'v')")
        header, value = header.split(':', 1)
        self.headers.add(header, value)

    def disableHttpCaching(self, level=1):
        """ Prevent caching of pages that should not be cached.

        level == 1 means disabling caching when we have a cookie set
        level == 2 means completely disabling caching (used by Page*Editor)

        This is important to prevent caches break acl by providing one
        user pages meant to be seen only by another user, when both users
        share the same caching proxy.

        AVOID using no-cache and no-store for attachments as it is completely broken on IE!

        Details: http://support.microsoft.com/support/kb/articles/Q234/0/67.ASP
        """
        if level == 1 and self.headers.get('Pragma') == 'no-cache':
            return

        if level == 1:
            self.headers['Cache-Control'] = 'private, must-revalidate, max-age=10'
        elif level == 2:
            self.headers['Cache-Control'] = 'no-cache'
            self.headers['Pragma'] = 'no-cache'
        self.request.expires = time.time() - 3600 * 24 * 365

    def http_redirect(self, url, code=302):
        """ Raise a simple redirect exception. """
        # werkzeug >= 0.6 does iri-to-uri transform if it gets unicode, but our
        # url is already url-quoted, so we better give it str to have same behaviour
        # with werkzeug 0.5.x and 0.6.x:
        url = str(url) # if url is unicode, it should contain ascii chars only
        abort(redirect(url, code=code))

    def http_user_agent(self):
        return self.environ.get('HTTP_USER_AGENT', '')
    http_user_agent = EnvironProxy(http_user_agent)

    def http_referer(self):
        return self.environ.get('HTTP_REFERER', '')
    http_referer = EnvironProxy(http_referer)

    # the output related methods
    def write(self, *data):
        """ Write to output stream. """
        self.request.out_stream.writelines(data)

    def redirectedOutput(self, function, *args, **kw):
        """ Redirect output during function, return redirected output """
        buf = StringIO.StringIO()
        self.redirect(buf)
        try:
            function(*args, **kw)
        finally:
            self.redirect()
        text = buf.getvalue()
        buf.close()
        return text

    def redirect(self, file=None):
        """ Redirect output to file, or restore saved output """
        if file:
            self.writestack.append(self.write)
            self.write = file.write
        else:
            self.write = self.writestack.pop()

    def send_file(self, fileobj, bufsize=8192, do_flush=None):
        """ Send a file to the output stream.

        @param fileobj: a file-like object (supporting read, close)
        @param bufsize: size of chunks to read/write
        @param do_flush: call flush after writing?
        """
        def simple_wrapper(fileobj, bufsize):
            return iter(lambda: fileobj.read(bufsize), '')
        file_wrapper = self.environ.get('wsgi.file_wrapper', simple_wrapper)
        self.request.direct_passthrough = True
        self.request.response = file_wrapper(fileobj, bufsize)
        raise MoinMoinFinish('sent file')

    # fully deprecated functions, with warnings
    def getScriptname(self):
        warnings.warn(
            "request.getScriptname() is deprecated, please use the request's script_root property.",
            DeprecationWarning)
        return self.request.script_root

    def getBaseURL(self):
        warnings.warn(
            "request.getBaseURL() is deprecated, please use the request's "
            "url_root property or the abs_href object if urls should be generated.",
            DeprecationWarning)
        return self.request.url_root

    def getQualifiedURL(self, uri=''):
        """ Return an absolute URL starting with schema and host.

        Already qualified urls are returned unchanged.

        @param uri: server rooted uri e.g /scriptname/pagename.
                    It must start with a slash. Must be ascii and url encoded.
        """
        import urlparse
        scheme = urlparse.urlparse(uri)[0]
        if scheme:
            return uri

        host_url = self.request.host_url.rstrip('/')
        result = "%s%s" % (host_url, uri)

        # This might break qualified urls in redirects!
        # e.g. mapping 'http://netloc' -> '/'
        result = wikiutil.mapURL(self, result)
        return result

class AuxilaryMixin(object):
    """
    Mixin for diverse attributes and methods that aren't clearly assignable
    to a particular phase of the request.
    """

    # several attributes used by other code to hold state across calls
    _fmt_hd_counters = EnvironProxy('_fmt_hd_counters')
    parsePageLinks_running = EnvironProxy('parsePageLinks_running', lambda o: {})
    mode_getpagelinks = EnvironProxy('mode_getpagelinks', 0)

    pragma = EnvironProxy('pragma', lambda o: {})
    _login_messages = EnvironProxy('_login_messages', lambda o: [])
    _login_multistage = EnvironProxy('_login_multistage', None)
    _login_multistage_name = EnvironProxy('_login_multistage_name', None)
    _setuid_real_user = EnvironProxy('_setuid_real_user', None)
    pages = EnvironProxy('pages', lambda o: {})

    def uid_generator(self):
        pagename = None
        if hasattr(self, 'page') and hasattr(self.page, 'page_name'):
            pagename = self.page.page_name
        return UniqueIDGenerator(pagename=pagename)
    uid_generator = EnvironProxy(uid_generator)

    def dicts(self):
        """ Lazy initialize the dicts on the first access """
        dicts = self.cfg.dicts(self)
        return dicts
    dicts = EnvironProxy(dicts)

    def groups(self):
        """ Lazy initialize the groups on the first access """
        groups = self.cfg.groups(self)
        return groups
    groups = EnvironProxy(groups)

    def reset(self):
        self.current_lang = self.cfg.language_default
        if hasattr(self, '_fmt_hd_counters'):
            del self._fmt_hd_counters
        if hasattr(self, 'uid_generator'):
            del self.uid_generator

    def getPragma(self, key, defval=None):
        """ Query a pragma value (#pragma processing instruction)

            Keys are not case-sensitive.
        """
        return self.pragma.get(key.lower(), defval)

    def setPragma(self, key, value):
        """ Set a pragma value (#pragma processing instruction)

            Keys are not case-sensitive.
        """
        self.pragma[key.lower()] = value

class XMLRPCContext(HTTPContext, AuxilaryMixin):
    """ Context to act during a XMLRPC request. """

class AllContext(HTTPContext, AuxilaryMixin):
    """ Catchall context to be able to quickly test old Moin code. """

class ScriptContext(AllContext):
    """ Context to act in scripting environments (e.g. former request_cli).

    For input, sys.stdin is used as 'wsgi.input', output is written directly
    to sys.stdout though.
    """
    def __init__(self, url=None, pagename=''):
        if url is None:
            url = 'http://localhost:0/' # just some somehow valid dummy URL
        environ = create_environ(base_url=url) # XXX not sure about base_url, but makes "make underlay" work
        environ['HTTP_USER_AGENT'] = 'CLI/Script'
        environ['wsgi.input'] = sys.stdin
        request = Request(environ)
        super(ScriptContext, self).__init__(request)
        from MoinMoin import wsgiapp
        wsgiapp.init(self)

    def write(self, *data):
        for d in data:
            if isinstance(d, unicode):
                d = d.encode(config.charset)
            else:
                d = str(d)
            sys.stdout.write(d)
