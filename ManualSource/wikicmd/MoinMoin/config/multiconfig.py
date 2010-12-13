# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Multiple configuration handler and Configuration defaults class

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2005-2008 MoinMoin:ThomasWaldmann.
                2008      MoinMoin:JohannesBerg
    @license: GNU GPL, see COPYING for details.
"""

import re
import os
import sys
import time

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin import config, error, util, wikiutil, web
from MoinMoin import datastruct
from MoinMoin.auth import MoinAuth
import MoinMoin.auth as authmodule
import MoinMoin.events as events
from MoinMoin.events import PageChangedEvent, PageRenamedEvent
from MoinMoin.events import PageDeletedEvent, PageCopiedEvent
from MoinMoin.events import PageRevertedEvent, FileAttachedEvent
import MoinMoin.web.session
from MoinMoin.packages import packLine
from MoinMoin.security import AccessControlList
from MoinMoin.support.python_compatibility import set

_url_re_cache = None
_farmconfig_mtime = None
_config_cache = {}


def _importConfigModule(name):
    """ Import and return configuration module and its modification time

    Handle all errors except ImportError, because missing file is not
    always an error.

    @param name: module name
    @rtype: tuple
    @return: module, modification time
    """
    try:
        module = __import__(name, globals(), {})
        mtime = os.path.getmtime(module.__file__)
    except ImportError:
        raise
    except IndentationError, err:
        logging.exception('Your source code / config file is not correctly indented!')
        msg = """IndentationError: %(err)s

The configuration files are Python modules. Therefore, whitespace is
important. Make sure that you use only spaces, no tabs are allowed here!
You have to use four spaces at the beginning of the line mostly.
""" % {
    'err': err,
}
        raise error.ConfigurationError(msg)
    except Exception, err:
        logging.exception('An exception happened.')
        msg = '%s: %s' % (err.__class__.__name__, str(err))
        raise error.ConfigurationError(msg)
    return module, mtime


def _url_re_list():
    """ Return url matching regular expression

    Import wikis list from farmconfig on the first call and compile the
    regexes. Later just return the cached regex list.

    @rtype: list of tuples of (name, compiled re object)
    @return: url to wiki config name matching list
    """
    global _url_re_cache, _farmconfig_mtime
    if _url_re_cache is None:
        try:
            farmconfig, _farmconfig_mtime = _importConfigModule('farmconfig')
        except ImportError, err:
            if 'farmconfig' in str(err):
                # we failed importing farmconfig
                logging.debug("could not import farmconfig, mapping all URLs to wikiconfig")
                _farmconfig_mtime = 0
                _url_re_cache = [('wikiconfig', re.compile(r'.')), ] # matches everything
            else:
                # maybe there was a failing import statement inside farmconfig
                raise
        else:
            logging.info("using farm config: %s" % os.path.abspath(farmconfig.__file__))
            try:
                cache = []
                for name, regex in farmconfig.wikis:
                    cache.append((name, re.compile(regex)))
                _url_re_cache = cache
            except AttributeError:
                logging.error("required 'wikis' list missing in farmconfig")
                msg = """
Missing required 'wikis' list in 'farmconfig.py'.

If you run a single wiki you do not need farmconfig.py. Delete it and
use wikiconfig.py.
"""
                raise error.ConfigurationError(msg)
    return _url_re_cache


def _makeConfig(name):
    """ Create and return a config instance

    Timestamp config with either module mtime or farmconfig mtime. This
    mtime can be used later to invalidate older caches.

    @param name: module name
    @rtype: DefaultConfig sub class instance
    @return: new configuration instance
    """
    global _farmconfig_mtime
    try:
        module, mtime = _importConfigModule(name)
        configClass = getattr(module, 'Config')
        cfg = configClass(name)
        cfg.cfg_mtime = max(mtime, _farmconfig_mtime)
        logging.info("using wiki config: %s" % os.path.abspath(module.__file__))
    except ImportError, err:
        logging.exception('Could not import.')
        msg = """ImportError: %(err)s

Check that the file is in the same directory as the server script. If
it is not, you must add the path of the directory where the file is
located to the python path in the server script. See the comments at
the top of the server script.

Check that the configuration file name is either "wikiconfig.py" or the
module name specified in the wikis list in farmconfig.py. Note that the
module name does not include the ".py" suffix.
""" % {
    'err': err,
}
        raise error.ConfigurationError(msg)
    except AttributeError, err:
        logging.exception('An exception occured.')
        msg = """AttributeError: %(err)s

Could not find required "Config" class in "%(name)s.py".

This might happen if you are trying to use a pre 1.3 configuration file, or
made a syntax or spelling error.

Another reason for this could be a name clash. It is not possible to have
config names like e.g. stats.py - because that collides with MoinMoin/stats/ -
have a look into your MoinMoin code directory what other names are NOT
possible.

Please check your configuration file. As an example for correct syntax,
use the wikiconfig.py file from the distribution.
""" % {
    'name': name,
    'err': err,
}
        raise error.ConfigurationError(msg)

    return cfg


def _getConfigName(url):
    """ Return config name for url or raise """
    for name, regex in _url_re_list():
        match = regex.match(url)
        if match:
            return name
    raise error.NoConfigMatchedError


def getConfig(url):
    """ Return cached config instance for url or create new one

    If called by many threads in the same time multiple config
    instances might be created. The first created item will be
    returned, using dict.setdefault.

    @param url: the url from request, possibly matching specific wiki
    @rtype: DefaultConfig subclass instance
    @return: config object for specific wiki
    """
    cfgName = _getConfigName(url)
    try:
        cfg = _config_cache[cfgName]
    except KeyError:
        cfg = _makeConfig(cfgName)
        cfg = _config_cache.setdefault(cfgName, cfg)
    return cfg


# This is a way to mark some text for the gettext tools so that they don't
# get orphaned. See http://www.python.org/doc/current/lib/node278.html.
def _(text):
    return text


class CacheClass:
    """ just a container for stuff we cache """
    pass


class ConfigFunctionality(object):
    """ Configuration base class with config class behaviour.

        This class contains the functionality for the DefaultConfig
        class for the benefit of the WikiConfig macro.
    """

    # attributes of this class that should not be shown
    # in the WikiConfig() macro.
    cfg_mtime = None
    siteid = None
    cache = None
    mail_enabled = None
    jabber_enabled = None
    auth_can_logout = None
    auth_have_login = None
    auth_login_inputs = None
    _site_plugin_lists = None
    _iwid = None
    _iwid_full = None
    xapian_searchers = None
    moinmoin_dir = None
    # will be lazily loaded by interwiki code when needed (?)
    shared_intermap_files = None

    def __init__(self, siteid):
        """ Init Config instance """
        self.siteid = siteid
        self.cache = CacheClass()

        from MoinMoin.Page import ItemCache
        self.cache.meta = ItemCache('meta')
        self.cache.pagelists = ItemCache('pagelists')

        if self.config_check_enabled:
            self._config_check()

        # define directories
        self.moinmoin_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir))
        data_dir = os.path.normpath(self.data_dir)
        self.data_dir = data_dir
        for dirname in ('user', 'cache', 'plugin'):
            name = dirname + '_dir'
            if not getattr(self, name, None):
                setattr(self, name, os.path.abspath(os.path.join(data_dir, dirname)))
        # directories below cache_dir (using __dirname__ to avoid conflicts)
        for dirname in ('session', ):
            name = dirname + '_dir'
            if not getattr(self, name, None):
                setattr(self, name, os.path.abspath(os.path.join(self.cache_dir, '__%s__' % dirname)))

        # Try to decode certain names which allow unicode
        self._decode()

        # After that, pre-compile some regexes
        self.cache.page_category_regex = re.compile(self.page_category_regex, re.UNICODE)
        self.cache.page_dict_regex = re.compile(self.page_dict_regex, re.UNICODE)
        self.cache.page_group_regex = re.compile(self.page_group_regex, re.UNICODE)
        self.cache.page_template_regex = re.compile(self.page_template_regex, re.UNICODE)

        # the ..._regexact versions only match if nothing is left (exact match)
        self.cache.page_category_regexact = re.compile(u'^%s$' % self.page_category_regex, re.UNICODE)
        self.cache.page_dict_regexact = re.compile(u'^%s$' % self.page_dict_regex, re.UNICODE)
        self.cache.page_group_regexact = re.compile(u'^%s$' % self.page_group_regex, re.UNICODE)
        self.cache.page_template_regexact = re.compile(u'^%s$' % self.page_template_regex, re.UNICODE)

        self.cache.ua_spiders = self.ua_spiders and re.compile(self.ua_spiders, re.IGNORECASE)

        self._check_directories()

        if not isinstance(self.superuser, list):
            msg = """The superuser setting in your wiki configuration is not a list
                     (e.g. ['Sample User', 'AnotherUser']).
                     Please change it in your wiki configuration and try again."""
            raise error.ConfigurationError(msg)

        # moin < 1.9 used cookie_lifetime = <float> (but converted it to int) for logged-in users and
        # anonymous_session_lifetime = <float> or None for anon users
        # moin >= 1.9 uses cookie_lifetime = (<float>, <float>) - first is anon, second is logged-in
        if not (isinstance(self.cookie_lifetime, tuple) and len(self.cookie_lifetime) == 2):
            logging.error("wiki configuration has an invalid setting: " +
                          "cookie_lifetime = %r" % (self.cookie_lifetime, ))
            try:
                anon_lifetime = self.anonymous_session_lifetime
                logging.warning("wiki configuration has an unsupported setting: " +
                                "anonymous_session_lifetime = %r - " % anon_lifetime +
                                "please remove it.")
                if anon_lifetime is None:
                    anon_lifetime = 0
                anon_lifetime = float(anon_lifetime)
            except:
                # if anything goes wrong, use default value
                anon_lifetime = 0
            try:
                logged_in_lifetime = int(self.cookie_lifetime)
            except:
                # if anything goes wrong, use default value
                logged_in_lifetime = 12
            self.cookie_lifetime = (anon_lifetime, logged_in_lifetime)
            logging.warning("using cookie_lifetime = %r - " % (self.cookie_lifetime, ) +
                            "please fix your wiki configuration.")

        self._loadPluginModule()

        # Preparse user dicts
        self._fillDicts()

        # Normalize values
        self.language_default = self.language_default.lower()

        # Use site name as default name-logo
        if self.logo_string is None:
            self.logo_string = self.sitename

        # Check for needed modules

        # FIXME: maybe we should do this check later, just before a
        # chart is needed, maybe in the chart module, instead doing it
        # for each request. But this require a large refactoring of
        # current code.
        if self.chart_options:
            try:
                import gdchart
            except ImportError:
                self.chart_options = None

        # "Render As Docbook" requires python-xml.
        if 'RenderAsDocbook' not in self.actions_excluded:
            try:
                from xml.dom.ext.reader import Sax
            except ImportError:
                # this will also remove it from the actions menu:
                self.actions_excluded.append('RenderAsDocbook')

        # 'setuid' special auth method auth method can log out
        self.auth_can_logout = ['setuid']
        self.auth_login_inputs = []
        found_names = []
        for auth in self.auth:
            if not auth.name:
                raise error.ConfigurationError("Auth methods must have a name.")
            if auth.name in found_names:
                raise error.ConfigurationError("Auth method names must be unique.")
            found_names.append(auth.name)
            if auth.logout_possible and auth.name:
                self.auth_can_logout.append(auth.name)
            for input in auth.login_inputs:
                if not input in self.auth_login_inputs:
                    self.auth_login_inputs.append(input)
        self.auth_have_login = len(self.auth_login_inputs) > 0
        self.auth_methods = found_names

        # internal dict for plugin `modules' lists
        self._site_plugin_lists = {}

        # we replace any string placeholders with config values
        # e.g u'%(page_front_page)s' % self
        self.navi_bar = [elem % self for elem in self.navi_bar]

        # check if python-xapian is installed
        if self.xapian_search:
            try:
                import xapian
            except ImportError, err:
                self.xapian_search = False
                logging.error("xapian_search was auto-disabled because python-xapian is not installed [%s]." % str(err))

        # list to cache xapian searcher objects
        self.xapian_searchers = []

        # check if mail is possible and set flag:
        self.mail_enabled = (self.mail_smarthost is not None or self.mail_sendmail is not None) and self.mail_from
        self.mail_enabled = self.mail_enabled and True or False

        # check if jabber bot is available and set flag:
        self.jabber_enabled = self.notification_bot_uri is not None

        # if we are to use the jabber bot, instantiate a server object for future use
        if self.jabber_enabled:
            from xmlrpclib import Server
            self.notification_server = Server(self.notification_bot_uri, )

        # Cache variables for the properties below
        self._iwid = self._iwid_full = self._meta_dict = None

        self.cache.acl_rights_before = AccessControlList(self, [self.acl_rights_before])
        self.cache.acl_rights_default = AccessControlList(self, [self.acl_rights_default])
        self.cache.acl_rights_after = AccessControlList(self, [self.acl_rights_after])

        action_prefix = self.url_prefix_action
        if action_prefix is not None and action_prefix.endswith('/'): # make sure there is no trailing '/'
            self.url_prefix_action = action_prefix[:-1]

        if self.url_prefix_local is None:
            self.url_prefix_local = self.url_prefix_static

        if self.url_prefix_fckeditor is None:
            self.url_prefix_fckeditor = self.url_prefix_local + '/applets/FCKeditor'

        if self.secrets is None:  # admin did not setup a real secret, so make up something
            self.secrets = self.calc_secrets()

        secret_key_names = ['action/cache', 'wikiutil/tickets', 'xmlrpc/ProcessMail', 'xmlrpc/RemoteScript', ]
        if self.jabber_enabled:
            secret_key_names.append('jabberbot')

        secret_min_length = 10
        if isinstance(self.secrets, str):
            if len(self.secrets) < secret_min_length:
                raise error.ConfigurationError("The secrets = '...' wiki config setting is a way too short string (minimum length is %d chars)!" % (
                    secret_min_length))
            # for lazy people: set all required secrets to same value
            secrets = {}
            for key in secret_key_names:
                secrets[key] = self.secrets
            self.secrets = secrets

        # we check if we have all secrets we need and that they have minimum length
        for secret_key_name in secret_key_names:
            try:
                secret = self.secrets[secret_key_name]
                if len(secret) < secret_min_length:
                    raise ValueError
            except (KeyError, ValueError):
                raise error.ConfigurationError("You must set a (at least %d chars long) secret string for secrets['%s']!" % (
                    secret_min_length, secret_key_name))

    def calc_secrets(self):
        """ make up some 'secret' using some config values """
        varnames = ['data_dir', 'data_underlay_dir', 'language_default',
                    'mail_smarthost', 'mail_from', 'page_front_page',
                    'theme_default', 'sitename', 'logo_string',
                    'interwikiname', 'user_homewiki', 'acl_rights_before', ]
        secret = ''
        for varname in varnames:
            var = getattr(self, varname, None)
            if isinstance(var, (str, unicode)):
                secret += repr(var)
        return secret

    _meta_dict = None
    def load_meta_dict(self):
        """ The meta_dict contains meta data about the wiki instance. """
        if self._meta_dict is None:
            self._meta_dict = wikiutil.MetaDict(os.path.join(self.data_dir, 'meta'), self.cache_dir)
        return self._meta_dict
    meta_dict = property(load_meta_dict)

    # lazily load iwid(_full)
    def make_iwid_property(attr):
        def getter(self):
            if getattr(self, attr, None) is None:
                self.load_IWID()
            return getattr(self, attr)
        return property(getter)
    iwid = make_iwid_property("_iwid")
    iwid_full = make_iwid_property("_iwid_full")

    # lazily create a list of event handlers
    _event_handlers = None
    def make_event_handlers_prop():
        def getter(self):
            if self._event_handlers is None:
                self._event_handlers = events.get_handlers(self)
            return self._event_handlers

        def setter(self, new_handlers):
            self._event_handlers = new_handlers

        return property(getter, setter)
    event_handlers = make_event_handlers_prop()

    def load_IWID(self):
        """ Loads the InterWikiID of this instance. It is used to identify the instance
            globally.
            The IWID is available as cfg.iwid
            The full IWID containing the interwiki name is available as cfg.iwid_full
            This method is called by the property.
        """
        try:
            iwid = self.meta_dict['IWID']
        except KeyError:
            iwid = util.random_string(16).encode("hex") + "-" + str(int(time.time()))
            self.meta_dict['IWID'] = iwid
            self.meta_dict.sync()

        self._iwid = iwid
        if self.interwikiname is not None:
            self._iwid_full = packLine([iwid, self.interwikiname])
        else:
            self._iwid_full = packLine([iwid])

    def _config_check(self):
        """ Check namespace and warn about unknown names

        Warn about names which are not used by DefaultConfig, except
        modules, classes, _private or __magic__ names.

        This check is disabled by default, when enabled, it will show an
        error message with unknown names.
        """
        unknown = ['"%s"' % name for name in dir(self)
                  if not name.startswith('_') and
                  name not in DefaultConfig.__dict__ and
                  not isinstance(getattr(self, name), (type(sys), type(DefaultConfig)))]
        if unknown:
            msg = """
Unknown configuration options: %s.

For more information, visit HelpOnConfiguration. Please check your
configuration for typos before requesting support or reporting a bug.
""" % ', '.join(unknown)
            raise error.ConfigurationError(msg)

    def _decode(self):
        """ Try to decode certain names, ignore unicode values

        Try to decode str using utf-8. If the decode fail, raise FatalError.

        Certain config variables should contain unicode values, and
        should be defined with u'text' syntax. Python decode these if
        the file have a 'coding' line.

        This will allow utf-8 users to use simple strings using, without
        using u'string'. Other users will have to use u'string' for
        these names, because we don't know what is the charset of the
        config files.
        """
        charset = 'utf-8'
        message = u"""
"%(name)s" configuration variable is a string, but should be
unicode. Use %(name)s = u"value" syntax for unicode variables.

Also check your "-*- coding -*-" line at the top of your configuration
file. It should match the actual charset of the configuration file.
"""

        decode_names = (
            'sitename', 'interwikiname', 'user_homewiki', 'logo_string', 'navi_bar',
            'page_front_page', 'page_category_regex', 'page_dict_regex',
            'page_group_regex', 'page_template_regex', 'page_license_page',
            'page_local_spelling_words', 'acl_rights_default',
            'acl_rights_before', 'acl_rights_after', 'mail_from'
            )

        for name in decode_names:
            attr = getattr(self, name, None)
            if attr:
                # Try to decode strings
                if isinstance(attr, str):
                    try:
                        setattr(self, name, unicode(attr, charset))
                    except UnicodeError:
                        raise error.ConfigurationError(message %
                                                       {'name': name})
                # Look into lists and try to decode strings inside them
                elif isinstance(attr, list):
                    for i in xrange(len(attr)):
                        item = attr[i]
                        if isinstance(item, str):
                            try:
                                attr[i] = unicode(item, charset)
                            except UnicodeError:
                                raise error.ConfigurationError(message %
                                                               {'name': name})

    def _check_directories(self):
        """ Make sure directories are accessible

        Both data and underlay should exists and allow read, write and
        execute.
        """
        mode = os.F_OK | os.R_OK | os.W_OK | os.X_OK
        for attr in ('data_dir', 'data_underlay_dir'):
            path = getattr(self, attr)

            # allow an empty underlay path or None
            if attr == 'data_underlay_dir' and not path:
                continue

            path_pages = os.path.join(path, "pages")
            if not (os.path.isdir(path_pages) and os.access(path_pages, mode)):
                msg = """
%(attr)s "%(path)s" does not exist, or has incorrect ownership or
permissions.

Make sure the directory and the subdirectory "pages" are owned by the web
server and are readable, writable and executable by the web server user
and group.

It is recommended to use absolute paths and not relative paths. Check
also the spelling of the directory name.
""" % {'attr': attr, 'path': path, }
                raise error.ConfigurationError(msg)

    def _loadPluginModule(self):
        """
        import all plugin modules

        To be able to import plugin from arbitrary path, we have to load
        the base package once using imp.load_module. Later, we can use
        standard __import__ call to load plugins in this package.

        Since each configured plugin path has unique plugins, we load the
        plugin packages as "moin_plugin_<sha1(path)>.plugin".
        """
        import imp
        from MoinMoin.support.python_compatibility import hash_new

        plugin_dirs = [self.plugin_dir] + self.plugin_dirs
        self._plugin_modules = []

        try:
            # Lock other threads while we check and import
            imp.acquire_lock()
            try:
                for pdir in plugin_dirs:
                    csum = 'p_%s' % hash_new('sha1', pdir).hexdigest()
                    modname = '%s.%s' % (self.siteid, csum)
                    # If the module is not loaded, try to load it
                    if not modname in sys.modules:
                        # Find module on disk and try to load - slow!
                        abspath = os.path.abspath(pdir)
                        parent_dir, pname = os.path.split(abspath)
                        fp, path, info = imp.find_module(pname, [parent_dir])
                        try:
                            # Load the module and set in sys.modules
                            module = imp.load_module(modname, fp, path, info)
                            setattr(sys.modules[self.siteid], 'csum', module)
                        finally:
                            # Make sure fp is closed properly
                            if fp:
                                fp.close()
                    if modname not in self._plugin_modules:
                        self._plugin_modules.append(modname)
            finally:
                imp.release_lock()
        except ImportError, err:
            msg = """
Could not import plugin package "%(path)s" because of ImportError:
%(err)s.

Make sure your data directory path is correct, check permissions, and
that the data/plugin directory has an __init__.py file.
""" % {
    'path': pdir,
    'err': str(err),
}
            raise error.ConfigurationError(msg)

    def _fillDicts(self):
        """ fill config dicts

        Fills in missing dict keys of derived user config by copying
        them from this base class.
        """
        # user checkbox defaults
        for key, value in DefaultConfig.user_checkbox_defaults.items():
            if key not in self.user_checkbox_defaults:
                self.user_checkbox_defaults[key] = value

    def __getitem__(self, item):
        """ Make it possible to access a config object like a dict """
        return getattr(self, item)


class DefaultConfig(ConfigFunctionality):
    """ Configuration base class with default config values
        (added below)
    """
    # Do not add anything into this class. Functionality must
    # be added above to avoid having the methods show up in
    # the WikiConfig macro. Settings must be added below to
    # the options dictionary.


def _default_password_checker(cfg, request, username, password):
    """ Check if a password is secure enough.
        We use a built-in check to get rid of the worst passwords.

        We do NOT use cracklib / python-crack here any more because it is
        not thread-safe (we experienced segmentation faults when using it).

        If you don't want to check passwords, use password_checker = None.

        @return: None if there is no problem with the password,
                 some unicode object with an error msg, if the password is problematic.
    """
    _ = request.getText
    # in any case, do a very simple built-in check to avoid the worst passwords
    if len(password) < 6:
        return _("Password is too short.")
    if len(set(password)) < 4:
        return _("Password has not enough different characters.")

    username_lower = username.lower()
    password_lower = password.lower()
    if username in password or password in username or \
       username_lower in password_lower or password_lower in username_lower:
        return _("Password is too easy (password contains name or name contains password).")

    keyboards = (ur"`1234567890-=qwertyuiop[]\asdfghjkl;'zxcvbnm,./", # US kbd
                 ur"^1234567890ß´qwertzuiopü+asdfghjklöä#yxcvbnm,.-", # german kbd
                ) # add more keyboards!
    for kbd in keyboards:
        rev_kbd = kbd[::-1]
        if password in kbd or password in rev_kbd or \
           password_lower in kbd or password_lower in rev_kbd:
            return _("Password is too easy (keyboard sequence).")
    return None


class DefaultExpression(object):
    def __init__(self, exprstr):
        self.text = exprstr
        self.value = eval(exprstr)


#
# Options that are not prefixed automatically with their
# group name, see below (at the options dict) for more
# information on the layout of this structure.
#
options_no_group_name = {
  # =========================================================================
  'attachment_extension': ("Mapping of attachment extensions to actions", None,
  (
   ('extensions_mapping',
       {'.tdraw': {'modify': 'twikidraw'},
        '.adraw': {'modify': 'anywikidraw'},
       }, "file extension -> do -> action"),
  )),
  # ==========================================================================
  'datastruct': ('Datastruct settings', None, (
    ('dicts', lambda cfg, request: datastruct.WikiDicts(request),
     "function f(cfg, request) that returns a backend which is used to access dicts definitions."),
    ('groups', lambda cfg, request: datastruct.WikiGroups(request),
     "function f(cfg, request) that returns a backend which is used to access groups definitions."),
  )),
  # ==========================================================================
  'session': ('Session settings', "Session-related settings, see HelpOnSessions.", (
    ('session_service', DefaultExpression('web.session.FileSessionService()'),
     "The session service."),
    ('cookie_name', None,
     'The variable part of the session cookie name. (None = determine from URL, siteidmagic = use siteid, any other string = use that)'),
    ('cookie_secure', None,
     'Use secure cookie. (None = auto-enable secure cookie for https, True = ever use secure cookie, False = never use secure cookie).'),
    ('cookie_httponly', False,
     'Use a httponly cookie that can only be used by the server, not by clientside scripts.'),
    ('cookie_domain', None,
     'Domain used in the session cookie. (None = do not specify domain).'),
    ('cookie_path', None,
     'Path used in the session cookie (None = auto-detect). Please only set if you know exactly what you are doing.'),
    ('cookie_lifetime', (0, 12),
     'Session lifetime [h] of (anonymous, logged-in) users (see HelpOnSessions for details).'),
  )),
  # ==========================================================================
  'auth': ('Authentication / Authorization / Security settings', None, (
    ('superuser', [],
     "List of trusted user names with wiki system administration super powers (not to be confused with ACL admin rights!). Used for e.g. software installation, language installation via SystemPagesSetup and more. See also HelpOnSuperUser."),
    ('auth', DefaultExpression('[MoinAuth()]'),
     "list of auth objects, to be called in this order (see HelpOnAuthentication)"),
    ('auth_methods_trusted', ['http', 'given', 'xmlrpc_applytoken'], # Note: 'http' auth method is currently just a redirect to 'given'
     'authentication methods for which users should be included in the special "Trusted" ACL group.'),
    ('secrets', None, """Either a long shared secret string used for multiple purposes or a dict {"purpose": "longsecretstring", ...} for setting up different shared secrets for different purposes. If you don't setup own secret(s), a secret string will be auto-generated from other config settings."""),
    ('DesktopEdition',
     False,
     "if True, give all local users special powers - ''only use this for a local desktop wiki!''"),
    ('SecurityPolicy',
     None,
     "Class object hook for implementing security restrictions or relaxations"),
    ('actions_excluded',
     ['xmlrpc',  # we do not want wiki admins unknowingly offering xmlrpc service
      'MyPages',  # only works when used with a non-default SecurityPolicy (e.g. autoadmin)
      'CopyPage',  # has questionable behaviour regarding subpages a user can't read, but can copy
     ],
     "Exclude unwanted actions (list of strings)"),

    ('allow_xslt', False,
     "if True, enables XSLT processing via 4Suite (note that this enables anyone with enough know-how to insert '''arbitrary HTML''' into your wiki, which is why it defaults to `False`)"),

    ('password_checker', DefaultExpression('_default_password_checker'),
     'checks whether a password is acceptable (default check is length >= 6, at least 4 different chars, no keyboard sequence, not username used somehow (you can switch this off by using `None`)'),

  )),
  # ==========================================================================
  'spam_leech_dos': ('Anti-Spam/Leech/DOS',
  'These settings help limiting ressource usage and avoiding abuse.',
  (
    ('hosts_deny', [], "List of denied IPs; if an IP ends with a dot, it denies a whole subnet (class A, B or C)"),
    ('surge_action_limits',
     {# allow max. <count> <action> requests per <dt> secs
        # action: (count, dt)
        'all': (30, 30), # all requests (except cache/AttachFile action) count for this limit
        'default': (30, 60), # default limit for actions without a specific limit
        'show': (30, 60),
        'recall': (10, 120),
        'raw': (20, 40),  # some people use this for css
        'diff': (30, 60),
        'fullsearch': (10, 120),
        'edit': (30, 300), # can be lowered after making preview different from edit
        'rss_rc': (1, 60),
        # The following actions are often used for images - to avoid pages with lots of images
        # (like photo galleries) triggering surge protection, we assign rather high limits:
        'AttachFile': (300, 30),
        'cache': (600, 30), # cache action is very cheap/efficient
     },
     "Surge protection tries to deny clients causing too much load/traffic, see HelpOnConfiguration/SurgeProtection."),
    ('surge_lockout_time', 3600, "time [s] someone gets locked out when ignoring the warnings"),

    ('textchas', None,
     "Spam protection setup using site-specific questions/answers, see HelpOnSpam."),
    ('textchas_disabled_group', None,
     "Name of a group of trusted users who do not get asked !TextCha questions."),

    ('antispam_master_url', "http://master.moinmo.in/?action=xmlrpc2",
     "where antispam security policy fetches spam pattern updates (if it is enabled)"),

    # a regex of HTTP_USER_AGENTS that should be excluded from logging
    # and receive a FORBIDDEN for anything except viewing a page
    # list must not contain 'java' because of twikidraw wanting to save drawing uses this useragent
    ('ua_spiders',
     ('archiver|cfetch|charlotte|crawler|curl|gigabot|googlebot|heritrix|holmes|htdig|httrack|httpunit|'
      'intelix|jeeves|larbin|leech|libwww-perl|linkbot|linkmap|linkwalk|litefinder|mercator|'
      'microsoft.url.control|mirror| mj12bot|msnbot|msrbot|neomo|nutbot|omniexplorer|puf|robot|scooter|seekbot|'
      'sherlock|slurp|sitecheck|snoopy|spider|teleport|twiceler|voilabot|voyager|webreaper|wget|yeti'),
     "A regex of HTTP_USER_AGENTs that should be excluded from logging and are not allowed to use actions."),

    ('unzip_single_file_size', 2.0 * 1000 ** 2,
     "max. size of a single file in the archive which will be extracted [bytes]"),
    ('unzip_attachments_space', 200.0 * 1000 ** 2,
     "max. total amount of bytes can be used to unzip files [bytes]"),
    ('unzip_attachments_count', 101,
     "max. number of files which are extracted from the zip file"),
  )),
  # ==========================================================================
  'style': ('Style / Theme / UI related',
  'These settings control how the wiki user interface will look like.',
  (
    ('sitename', u'Untitled Wiki',
     "Short description of your wiki site, displayed below the logo on each page, and used in RSS documents as the channel title [Unicode]"),
    ('interwikiname', None, "unique and stable InterWiki name (prefix, moniker) of the site [Unicode], or None"),
    ('logo_string', None, "The wiki logo top of page, HTML is allowed (`<img>` is possible as well) [Unicode]"),
    ('html_pagetitle', None, "Allows you to set a specific HTML page title (if None, it defaults to the value of `sitename`)"),
    ('navi_bar', [u'RecentChanges', u'FindPage', u'HelpContents', ],
     'Most important page names. Users can add more names in their quick links in user preferences. To link to URL, use `u"[[url|link title]]"`, to use a shortened name for long page name, use `u"[[LongLongPageName|title]]"`. [list of Unicode strings]'),

    ('theme_default', 'modernized',
     "the name of the theme that is used by default (see HelpOnThemes)"),
    ('theme_force', False,
     "if True, do not allow to change the theme"),

    ('stylesheets', [],
     "List of tuples (media, csshref) to insert after theme css, before user css, see HelpOnThemes."),

    ('supplementation_page', False,
     "if True, show a link to the supplementation page in the theme"),
    ('supplementation_page_name', u'Discussion',
     "default name of the supplementation (sub)page [unicode]"),
    ('supplementation_page_template', u'DiscussionTemplate',
     "default template used for creation of the supplementation page [unicode]"),

    ('interwiki_preferred', [], "In dialogues, show those wikis at the top of the list."),
    ('sistersites', [], "list of tuples `('WikiName', 'sisterpagelist_fetch_url')`"),

    ('trail_size', 5,
     "Number of pages in the trail of visited pages"),

    ('page_footer1', '', "Custom HTML markup sent ''before'' the system footer."),
    ('page_footer2', '', "Custom HTML markup sent ''after'' the system footer."),
    ('page_header1', '', "Custom HTML markup sent ''before'' the system header / title area but after the body tag."),
    ('page_header2', '', "Custom HTML markup sent ''after'' the system header / title area (and body tag)."),

    ('changed_time_fmt', '%H:%M', "Time format used on Recent``Changes for page edits within the last 24 hours"),
    ('date_fmt', '%Y-%m-%d', "System date format, used mostly in Recent``Changes"),
    ('datetime_fmt', '%Y-%m-%d %H:%M:%S', 'Default format for dates and times (when the user has no preferences or chose the "default" date format)'),
    ('chart_options', None, "If you have gdchart, use something like chart_options = {'width': 720, 'height': 540}"),

    ('edit_bar', ['Edit', 'Comments', 'Discussion', 'Info', 'Subscribe', 'Quicklink', 'Attachments', 'ActionsMenu'],
     'list of edit bar entries'),
    ('history_count', (100, 200, 5, 10, 25, 50), "Number of revisions shown for info/history action (default_count_shown, max_count_shown, [other values shown as page size choices]). At least first two values (default and maximum) should be provided. If additional values are provided, user will be able to change number of items per page in the UI."),
    ('history_paging', True, "Enable paging functionality for info action's history display."),

    ('show_hosts', True,
     "if True, show host names and IPs. Set to False to hide them."),
    ('show_interwiki', False,
     "if True, let the theme display your interwiki name"),
    ('show_names', True,
     "if True, show user names in the revision history and on Recent``Changes. Set to False to hide them."),
    ('show_section_numbers', False,
     'show section numbers in headings by default'),
    ('show_timings', False, "show some timing values at bottom of a page"),
    ('show_version', False, "show moin's version at the bottom of a page"),
    ('show_rename_redirect', False, "if True, offer creation of redirect pages when renaming wiki pages"),

    ('packagepages_actions_excluded',
     ['setthemename',  # related to questionable theme stuff, see below
      'copythemefile', # maybe does not work, e.g. if no fs write permissions or real theme file path is unknown to moin
      'installplugin', # code installation, potentially dangerous
      'renamepage',    # dangerous with hierarchical acls
      'deletepage',    # dangerous with hierarchical acls
      'delattachment', # dangerous, no revisioning
     ],
     'list with excluded package actions (e.g. because they are dangerous / questionable)'),

    ('page_credits',
     [
       '<a href="http://moinmo.in/" title="This site uses the MoinMoin Wiki software.">MoinMoin Powered</a>',
       '<a href="http://moinmo.in/Python" title="MoinMoin is written in Python.">Python Powered</a>',
       '<a href="http://moinmo.in/GPL" title="MoinMoin is GPL licensed.">GPL licensed</a>',
       '<a href="http://validator.w3.org/check?uri=referer" title="Click here to validate this page.">Valid HTML 4.01</a>',
     ],
     'list with html fragments with logos or strings for crediting.'),

    # These icons will show in this order in the iconbar, unless they
    # are not relevant, e.g email icon when the wiki is not configured
    # for email.
    ('page_iconbar', ["up", "edit", "view", "diff", "info", "subscribe", "raw", "print", ],
     'list of icons to show in iconbar, valid values are only those in page_icons_table. Available only in classic theme.'),

    # Standard buttons in the iconbar
    ('page_icons_table',
     {
        # key           pagekey, querystr dict, title, icon-key
        'diff': ('page', {'action': 'diff'}, _("Diffs"), "diff"),
        'info': ('page', {'action': 'info'}, _("Info"), "info"),
        'edit': ('page', {'action': 'edit'}, _("Edit"), "edit"),
        'unsubscribe': ('page', {'action': 'unsubscribe'}, _("UnSubscribe"), "unsubscribe"),
        'subscribe': ('page', {'action': 'subscribe'}, _("Subscribe"), "subscribe"),
        'raw': ('page', {'action': 'raw'}, _("Raw"), "raw"),
        'xml': ('page', {'action': 'show', 'mimetype': 'text/xml'}, _("XML"), "xml"),
        'print': ('page', {'action': 'print'}, _("Print"), "print"),
        'view': ('page', {}, _("View"), "view"),
        'up': ('page_parent_page', {}, _("Up"), "up"),
     },
     "dict of {'iconname': (url, title, icon-img-key), ...}. Available only in classic theme."),

  )),
  # ==========================================================================
  'editor': ('Editor related', None, (
    ('editor_default', 'text', "Editor to use by default, 'text' or 'gui'"),
    ('editor_force', False, "if True, force using the default editor"),
    ('editor_ui', 'freechoice', "Editor choice shown on the user interface, 'freechoice' or 'theonepreferred'"),
    ('page_license_enabled', False, 'if True, show a license hint in page editor.'),
    ('page_license_page', u'WikiLicense', 'Page linked from the license hint. [Unicode]'),
    ('edit_locking', 'warn 10', "Editor locking policy: `None`, `'warn <timeout in minutes>'`, or `'lock <timeout in minutes>'`"),
    ('edit_ticketing', True, None),
    ('edit_rows', 20, "Default height of the edit box"),

  )),
  # ==========================================================================
  'paths': ('Paths', None, (
    ('data_dir', './data/', "Path to the data directory containing your (locally made) wiki pages."),
    ('data_underlay_dir', './underlay/', "Path to the underlay directory containing distribution system and help pages."),
    ('cache_dir', None, "Directory for caching, by default computed from `data_dir`/cache."),
    ('session_dir', None, "Directory for session storage, by default computed to be `cache_dir`/__session__."),
    ('user_dir', None, "Directory for user storage, by default computed to be `data_dir`/user."),
    ('plugin_dir', None, "Plugin directory, by default computed to be `data_dir`/plugin."),
    ('plugin_dirs', [], "Additional plugin directories."),

    ('docbook_html_dir', r"/usr/share/xml/docbook/stylesheet/nwalsh/html/",
     'Path to the directory with the Docbook to HTML XSLT files (optional, used by the docbook parser). The default value is correct for Debian Etch.'),
    ('shared_intermap', None,
     "Path to a file containing global InterWiki definitions (or a list of such filenames)"),
  )),
  # ==========================================================================
  'urls': ('URLs', None, (
    # includes the moin version number, so we can have a unlimited cache lifetime
    # for the static stuff. if stuff changes on version upgrade, url will change
    # immediately and we have no problem with stale caches.
    ('url_prefix_static', config.url_prefix_static,
     "used as the base URL for icons, css, etc. - includes the moin version number and changes on every release. This replaces the deprecated and sometimes confusing `url_prefix = '/wiki'` setting."),
    ('url_prefix_local', None,
     "used as the base URL for some Javascript - set this to a URL on same server as the wiki if your url_prefix_static points to a different server."),
    ('url_prefix_fckeditor', None,
     "used as the base URL for FCKeditor - similar to url_prefix_local, but just for FCKeditor."),

    ('url_prefix_action', None,
     "Use 'action' to enable action URL generation to be compatible with robots.txt. It will generate .../action/info/PageName?action=info then. Recommended for internet wikis."),

    ('notification_bot_uri', None, "URI of the Jabber notification bot."),

    ('url_mappings', {},
     "lookup table to remap URL prefixes (dict of {{{'prefix': 'replacement'}}}); especially useful in intranets, when whole trees of externally hosted documents move around"),

  )),
  # ==========================================================================
  'pages': ('Special page names', None, (
    ('page_front_page', u'LanguageSetup',
     "Name of the front page. We don't expect you to keep the default. Just read LanguageSetup in case you're wondering... [Unicode]"),

    # the following regexes should match the complete name when used in free text
    # the group 'all' shall match all, while the group 'key' shall match the key only
    # e.g. CategoryFoo -> group 'all' ==  CategoryFoo, group 'key' == Foo
    # moin's code will add ^ / $ at beginning / end when needed
    ('page_category_regex', ur'(?P<all>Category(?P<key>(?!Template)\S+))',
     'Pagenames exactly matching this regex are regarded as Wiki categories [Unicode]'),
    ('page_dict_regex', ur'(?P<all>(?P<key>\S+)Dict)',
     'Pagenames exactly matching this regex are regarded as pages containing variable dictionary definitions [Unicode]'),
    ('page_group_regex', ur'(?P<all>(?P<key>\S+)Group)',
     'Pagenames exactly matching this regex are regarded as pages containing group definitions [Unicode]'),
    ('page_template_regex', ur'(?P<all>(?P<key>\S+)Template)',
     'Pagenames exactly matching this regex are regarded as pages containing templates for new pages [Unicode]'),

    ('page_local_spelling_words', u'LocalSpellingWords',
     'Name of the page containing user-provided spellchecker words [Unicode]'),
  )),
  # ==========================================================================
  'user': ('User Preferences related', None, (
    ('quicklinks_default', [],
     'List of preset quicklinks for a newly created user accounts. Existing accounts are not affected by this option whereas changes in navi_bar do always affect existing accounts. Preset quicklinks can be removed by the user in the user preferences menu, navi_bar settings not.'),
    ('subscribed_pages_default', [],
     "List of pagenames used for presetting page subscriptions for newly created user accounts."),

    ('email_subscribed_events_default',
     [
        PageChangedEvent.__name__,
        PageRenamedEvent.__name__,
        PageDeletedEvent.__name__,
        PageCopiedEvent.__name__,
        PageRevertedEvent.__name__,
        FileAttachedEvent.__name__,
     ], None),
    ('jabber_subscribed_events_default', [], None),

    ('tz_offset', 0.0,
     "default time zone offset in hours from UTC"),

    ('userprefs_disabled', [],
     "Disable the listed user preferences plugins."),
  )),
  # ==========================================================================
  'various': ('Various', None, (
    ('bang_meta', True, 'if True, enable {{{!NoWikiName}}} markup'),
    ('caching_formats', ['text_html'], "output formats that are cached; set to [] to turn off caching (useful for development)"),

    ('config_check_enabled', False, "if True, check configuration for unknown settings."),

    ('default_markup', 'wiki', 'Default page parser / format (name of module in `MoinMoin.parser`)'),

    ('html_head', '', "Additional <HEAD> tags, see HelpOnThemes."),
    ('html_head_queries', '<meta name="robots" content="noindex,nofollow">\n',
     "Additional <HEAD> tags for requests with query strings, like actions."),
    ('html_head_posts', '<meta name="robots" content="noindex,nofollow">\n',
     "Additional <HEAD> tags for POST requests."),
    ('html_head_index', '<meta name="robots" content="index,follow">\n',
     "Additional <HEAD> tags for some few index pages."),
    ('html_head_normal', '<meta name="robots" content="index,nofollow">\n',
     "Additional <HEAD> tags for most normal pages."),

    ('language_default', 'en', "Default language for user interface and page content, see HelpOnLanguages."),
    ('language_ignore_browser', False, "if True, ignore user's browser language settings, see HelpOnLanguages."),

    ('log_remote_addr', True,
     "if True, log the remote IP address (and maybe hostname)."),
    ('log_reverse_dns_lookups', True,
     "if True, do a reverse DNS lookup on page SAVE. If your DNS is broken, set this to False to speed up SAVE."),
    ('log_timing', False,
     "if True, add timing infos to the log output to analyse load conditions"),

    # some dangerous mimetypes (we don't use "content-disposition: inline" for them when a user
    # downloads such attachments, because the browser might execute e.g. Javascript contained
    # in the HTML and steal your moin session cookie or do other nasty stuff)
    ('mimetypes_xss_protect',
     [
       'text/html',
       'application/x-shockwave-flash',
       'application/xhtml+xml',
     ],
     '"content-disposition: inline" isn\'t used for them when a user downloads such attachments'),

    ('mimetypes_embed',
     [
       'application/x-dvi',
       'application/postscript',
       'application/pdf',
       'application/ogg',
       'application/vnd.visio',
       'image/x-ms-bmp',
       'image/svg+xml',
       'image/tiff',
       'image/x-photoshop',
       'audio/mpeg',
       'audio/midi',
       'audio/x-wav',
       'video/fli',
       'video/mpeg',
       'video/quicktime',
       'video/x-msvideo',
       'chemical/x-pdb',
       'x-world/x-vrml',
     ],
     'mimetypes that can be embedded by the [[HelpOnMacros/EmbedObject|EmbedObject macro]]'),

    ('refresh', None,
     "refresh = (minimum_delay_s, targets_allowed) enables use of `#refresh 5 PageName` processing instruction, targets_allowed must be either `'internal'` or `'external'`"),
    ('rss_cache', 60, "suggested caching time for Recent''''''Changes RSS, in second"),

    ('search_results_per_page', 25, "Number of hits shown per page in the search results"),

    ('siteid', 'default', None),
  )),
}

#
# The 'options' dict carries default MoinMoin options. The dict is a
# group name to tuple mapping.
# Each group tuple consists of the following items:
#   group section heading, group help text, option list
#
# where each 'option list' is a tuple or list of option tuples
#
# each option tuple consists of
#   option name, default value, help text
#
# All the help texts will be displayed by the WikiConfigHelp() macro.
#
# Unlike the options_no_group_name dict, option names in this dict
# are automatically prefixed with "group name '_'" (i.e. the name of
# the group they are in and an underscore), e.g. the 'hierarchic'
# below creates an option called "acl_hierarchic".
#
# If you need to add a complex default expression that results in an
# object and should not be shown in the __repr__ form in WikiConfigHelp(),
# you can use the DefaultExpression class, see 'auth' above for example.
#
#
options = {
    'acl': ('Access control lists',
    'ACLs control who may do what, see HelpOnAccessControlLists.',
    (
      ('hierarchic', False, 'True to use hierarchical ACLs'),
      ('rights_default', u"Trusted:read,write,delete,revert Known:read,write,delete,revert All:read,write",
       "ACL used if no ACL is specified on the page"),
      ('rights_before', u"",
       "ACL that is processed before the on-page/default ACL"),
      ('rights_after', u"",
       "ACL that is processed after the on-page/default ACL"),
      ('rights_valid', ['read', 'write', 'delete', 'revert', 'admin'],
       "Valid tokens for right sides of ACL entries."),
    )),

    'xapian': ('Xapian search', "Configuration of the Xapian based indexed search, see HelpOnXapian.", (
      ('search', False,
       "True to enable the fast, indexed search (based on the Xapian search library)"),
      ('index_dir', None,
       "Directory where the Xapian search index is stored (None = auto-configure wiki local storage)"),
      ('stemming', False,
       "True to enable Xapian word stemmer usage for indexing / searching."),
      ('index_history', False,
       "True to enable indexing of non-current page revisions."),
    )),

    'user': ('Users / User settings', None, (
      ('email_unique', True,
       "if True, check email addresses for uniqueness and don't accept duplicates."),
      ('jid_unique', True,
       "if True, check Jabber IDs for uniqueness and don't accept duplicates."),

      ('homewiki', u'Self',
       "interwiki name of the wiki where the user home pages are located [Unicode] - useful if you have ''many'' users. You could even link to nonwiki \"user pages\" if the wiki username is in the target URL."),

      ('checkbox_fields',
       [
        ('mailto_author', lambda _: _('Publish my email (not my wiki homepage) in author info')),
        ('edit_on_doubleclick', lambda _: _('Open editor on double click')),
        ('remember_last_visit', lambda _: _('After login, jump to last visited page')),
        ('show_comments', lambda _: _('Show comment sections')),
        ('show_nonexist_qm', lambda _: _('Show question mark for non-existing pagelinks')),
        ('show_page_trail', lambda _: _('Show page trail')),
        ('show_toolbar', lambda _: _('Show icon toolbar')),
        ('show_topbottom', lambda _: _('Show top/bottom links in headings')),
        ('show_fancy_diff', lambda _: _('Show fancy diffs')),
        ('wikiname_add_spaces', lambda _: _('Add spaces to displayed wiki names')),
        ('remember_me', lambda _: _('Remember login information')),

        ('disabled', lambda _: _('Disable this account forever')),
        # if an account is disabled, it may be used for looking up
        # id -> username for page info and recent changes, but it
        # is not usable for the user any more:
       ],
       "Describes user preferences, see HelpOnConfiguration/UserPreferences."),

      ('checkbox_defaults',
       {
        'mailto_author': 0,
        'edit_on_doubleclick': 1,
        'remember_last_visit': 0,
        'show_comments': 0,
        'show_nonexist_qm': False,
        'show_page_trail': 1,
        'show_toolbar': 1,
        'show_topbottom': 0,
        'show_fancy_diff': 1,
        'wikiname_add_spaces': 0,
        'remember_me': 1,
       },
       "Defaults for user preferences, see HelpOnConfiguration/UserPreferences."),

      ('checkbox_disable', [],
       "Disable user preferences, see HelpOnConfiguration/UserPreferences."),

      ('checkbox_remove', [],
       "Remove user preferences, see HelpOnConfiguration/UserPreferences."),

      ('form_fields',
       [
        ('name', _('Name'), "text", "36", _("(Use FirstnameLastname)")),
        ('aliasname', _('Alias-Name'), "text", "36", ''),
        ('email', _('Email'), "text", "36", ''),
        ('jid', _('Jabber ID'), "text", "36", ''),
        ('css_url', _('User CSS URL'), "text", "40", _('(Leave it empty for disabling user CSS)')),
        ('edit_rows', _('Editor size'), "text", "3", ''),
       ],
       None),

      ('form_defaults',
       {# key: default - do NOT remove keys from here!
        'name': '',
        'aliasname': '',
        'password': '',
        'password2': '',
        'email': '',
        'jid': '',
        'css_url': '',
        'edit_rows': "20",
       },
       None),

      ('form_disable', [], "list of field names used to disable user preferences form fields"),

      ('form_remove', [], "list of field names used to remove user preferences form fields"),

      ('transient_fields',
       ['id', 'valid', 'may', 'auth_username', 'password', 'password2', 'auth_method', 'auth_attribs', ],
       "User object attributes that are not persisted to permanent storage (internal use)."),
    )),

    'openidrp': ('OpenID Relying Party',
        'These settings control the built-in OpenID Relying Party (client).',
    (
      ('allowed_op', [], "List of forced providers"),
    )),

    'openid_server': ('OpenID Server',
        'These settings control the built-in OpenID Identity Provider (server).',
    (
      ('enabled', False, "True to enable the built-in OpenID server."),
      ('restricted_users_group', None, "If set to a group name, the group members are allowed to use the wiki as an OpenID provider. (None = allow for all users)"),
      ('enable_user', False, "If True, the OpenIDUser processing instruction is allowed."),
    )),

    'mail': ('Mail settings',
        'These settings control outgoing and incoming email from and to the wiki.',
    (
      ('from', None, "Used as From: address for generated mail."),
      ('login', None, "'username userpass' for SMTP server authentication (None = don't use auth)."),
      ('smarthost', None, "Address of SMTP server to use for sending mail (None = don't use SMTP server)."),
      ('sendmail', None, "sendmail command to use for sending mail (None = don't use sendmail)"),

      ('import_subpage_template', u"$from-$date-$subject", "Create subpages using this template when importing mail."),
      ('import_pagename_search', ['subject', 'to', ], "Where to look for target pagename specification."),
      ('import_pagename_envelope', u"%s", "Use this to add some fixed prefix/postfix to the generated target pagename."),
      ('import_pagename_regex', r'\[\[([^\]]*)\]\]', "Regular expression used to search for target pagename specification."),
      ('import_wiki_addrs', [], "Target mail addresses to consider when importing mail"),
    )),

    'backup': ('Backup settings',
        'These settings control how the backup action works and who is allowed to use it.',
    (
      ('compression', 'gz', 'What compression to use for the backup ("gz" or "bz2").'),
      ('users', [], 'List of trusted user names who are allowed to get a backup.'),
      ('include', [], 'List of pathes to backup.'),
      ('exclude', lambda self, filename: False, 'Function f(self, filename) that tells whether a file should be excluded from backup. By default, nothing is excluded.'),
    )),
}

def _add_options_to_defconfig(opts, addgroup=True):
    for groupname in opts:
        group_short, group_doc, group_opts = opts[groupname]
        for name, default, doc in group_opts:
            if addgroup:
                name = groupname + '_' + name
            if isinstance(default, DefaultExpression):
                default = default.value
            setattr(DefaultConfig, name, default)

_add_options_to_defconfig(options)
_add_options_to_defconfig(options_no_group_name, False)

# remove the gettext pseudo function
del _

