# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - WSGI session handling

    To provide sessions, the MoinMoin WSGI application interacts with an
    object implementing the `SessionService` API. The interface is quite
    straight forward. For documentation of the expected methods, refer
    to the documentation of `SessionService` in this module.

    @copyright: 2008 MoinMoin:FlorianKrupicka,
                2009 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
import sys, os
from os import path
import time
import tempfile
import re
try:
    from cPickle import load, dump, HIGHEST_PROTOCOL
except ImportError:
    from pickle import load, dump, HIGHEST_PROTOCOL

from werkzeug.contrib.sessions import SessionStore, ModificationTrackingDict

from MoinMoin import config
from MoinMoin.util import filesys
rename = filesys.rename # use MoinMoin's rename until we have it in werkzeug 0.6.1

from MoinMoin import log
logging = log.getLogger(__name__)


# start copy from werkzeug 0.6 - directly import this, if we require >= 0.6:

class Session(ModificationTrackingDict):
    """Subclass of a dict that keeps track of direct object changes.  Changes
    in mutable structures are not tracked, for those you have to set
    `modified` to `True` by hand.
    """
    __slots__ = ModificationTrackingDict.__slots__ + ('sid', 'new')

    def __init__(self, data, sid, new=False):
        ModificationTrackingDict.__init__(self, data)
        self.sid = sid
        self.new = new

    @property
    def should_save(self):
        """True if the session should be saved.

        .. versionchanged:: 0.6
           By default the session is now only saved if the session is
           modified, not if it is new like it was before.
        """
        return self.modified

    def __repr__(self):
        return '<%s %s %s%s>' % (
            self.__class__.__name__,
            self.sid,
            dict.__repr__(self),
            self.should_save and '*' or ''
        )


#: used for temporary files by the filesystem session store
_fs_transaction_suffix = '.__wz_sess'

class FilesystemSessionStore(SessionStore):
    """Simple example session store that saves sessions on the filesystem.
    This store works best on POSIX systems and Windows Vista / Windows
    Server 2008 and newer.

    .. versionchanged:: 0.6
       `renew_missing` was added.  Previously this was considered `True`,
       now the default changed to `False` and it can be explicitly
       deactivated.

    :param path: the path to the folder used for storing the sessions.
                 If not provided the default temporary directory is used.
    :param filename_template: a string template used to give the session
                              a filename.  ``%s`` is replaced with the
                              session id.
    :param session_class: The session class to use.  Defaults to
                          :class:`Session`.
    :param renew_missing: set to `True` if you want the store to
                          give the user a new sid if the session was
                          not yet saved.
    """

    def __init__(self, path=None, filename_template='werkzeug_%s.sess',
                 session_class=None, renew_missing=False, mode=0644):
        SessionStore.__init__(self, session_class)
        if path is None:
            path = tempfile.gettempdir()
        self.path = path
        if isinstance(filename_template, unicode):
            filename_template = filename_template.encode(
                sys.getfilesystemencoding() or 'utf-8')
        assert not filename_template.endswith(_fs_transaction_suffix), \
            'filename templates may not end with %s' % _fs_transaction_suffix
        self.filename_template = filename_template
        self.renew_missing = renew_missing
        self.mode = mode

    def get_session_filename(self, sid):
        # out of the box, this should be a strict ASCII subset but
        # you might reconfigure the session object to have a more
        # arbitrary string.
        if isinstance(sid, unicode):
            sid = sid.encode(sys.getfilesystemencoding() or 'utf-8')
        return path.join(self.path, self.filename_template % sid)

    def save(self, session):
        fn = self.get_session_filename(session.sid)
        fd, tmp = tempfile.mkstemp(suffix=_fs_transaction_suffix,
                                   dir=self.path)
        f = os.fdopen(fd, 'wb')
        try:
            dump(dict(session), f, HIGHEST_PROTOCOL)
        finally:
            f.close()
        try:
            rename(tmp, fn)
            os.chmod(fn, self.mode)
        except (IOError, OSError):
            pass

    def delete(self, session):
        fn = self.get_session_filename(session.sid)
        try:
            os.unlink(fn)
        except OSError:
            pass

    def get(self, sid):
        if not self.is_valid_key(sid):
            return self.new()
        try:
            f = open(self.get_session_filename(sid), 'rb')
        except IOError:
            if self.renew_missing:
                return self.new()
            data = {}
        else:
            try:
                try:
                    data = load(f)
                except Exception:
                    data = {}
            finally:
                f.close()
        return self.session_class(data, sid, False)

    def list(self):
        """Lists all sessions in the store.

        .. versionadded:: 0.6
        """
        before, after = self.filename_template.split('%s', 1)
        filename_re = re.compile(r'%s(.{5,})%s$' % (re.escape(before),
                                                    re.escape(after)))
        result = []
        for filename in os.listdir(self.path):
            #: this is a session that is still being saved.
            if filename.endswith(_fs_transaction_suffix):
                continue
            match = filename_re.match(filename)
            if match is not None:
                result.append(match.group(1))
        return result

# end copy of werkzeug 0.6 code


class MoinSession(Session):
    """ Compatibility interface to Werkzeug-sessions for old Moin-code.

        is_new is DEPRECATED and will go away soon.
    """
    def _get_is_new(self):
        logging.warning("Deprecated use of MoinSession.is_new, please use .new")
        return self.new
    is_new = property(_get_is_new)


class SessionService(object):
    """
    A session service returns a session object given a request object and
    provides services like persisting sessions and cleaning up occasionally.
    """
    def get_session(self, request, sid=None):
        """ Return a session object pertaining to the particular request."""
        raise NotImplementedError

    def destroy_session(self, request, session):
        """ Destroy an existing session (make it unusable). """
        raise NotImplementedError

    def finalize(self, request, session):
        """
        If the service needs to do anything to the session and/or request,
        before it is sent back to the client, he can chose to do so here.
        Typical examples would be setting cookies for the client.
        """
        raise NotImplementedError

    def get_all_session_ids(self, request):
        """
        Return a list of all session ids known to the SessionService.
        """
        raise NotImplementedError


def _get_session_lifetime(request, userobj):
    """ Get session lifetime for the user object userobj
    Cookie lifetime in hours, can be fractional. First tuple element is for anonymous sessions,
    second tuple element is for logged-in sessions. For anonymous sessions,
    t=0 means that they are disabled, t>0 means that many hours.
    For logged-in sessions, t>0 means that many hours,
    or forever if user checked 'remember_me', t<0 means -t hours and
    ignore user 'remember_me' setting - you usually don't want to use t=0, it disables logged-in sessions."""
    lifetime = int(float(request.cfg.cookie_lifetime[userobj and userobj.valid]) * 3600)
    forever = 10 * 365 * 24 * 3600 # 10 years

    if userobj and userobj.valid and userobj.remember_me and lifetime > 0:
        return forever
    return abs(lifetime)


def get_cookie_name(request, name, usage, software='MOIN'):
    """
    Determine the full cookie name for some software (usually 'MOIN') using
    it for some usage (e.g. 'SESSION') for some wiki (or group of wikis)
    determined by name.

    Note:
    -----
    We do not use the path=... information in the cookie any more, because it can
    easily cause confusion if there are multiple cookies with same name, but
    different pathes (like e.g. / and /foo).

    Instead of using the cookie path, we use differently named cookies, so we get
    the right cookie no matter at what URL the wiki currently is "mounted".

    If name is None, we use some URL components to make up some name.
    For example the cookie name for the default desktop wiki: MOIN_SESSION_8080_ROOT

    If name is siteidmagic, we just use cfg.siteid, which is unique within a wiki farm
    created by a single farmconfig. If you only run ONE(!) wikiconfig wiki, it
    is also unique, of course, but not if you run multiple wikiconfig wikis under
    same domain.

    If name is not None (and not 'siteidmagic'), we just use the given name (you
    want to use that to share stuff between several wikis - just give same name
    and it will use the same cookie. same thing if you don't want to share, just
    give a different name then [e.g. if cfg.siteid or 'siteidmagic' doesn't work
    for you]).

    Moving a wiki to a different URL will break all sessions. Exchanging URLs
    of wikis might lead to confusion (requiring the client to purge the cookies).
    """
    if name is None:
        url_components = [
            # cookies do not store the port, thus we add it to the cookie name:
            request.environ['SERVER_PORT'],
            # we always store path=/ into cookie, thus we add the path to the name:
            ('ROOT' + request.script_root).replace('/', '_'),
        ]
        name = '_'.join(url_components)

    elif name is 'siteidmagic':
        name = request.cfg.siteid  # == config name, unique per farm

    return "%s_%s_%s" % (software, usage, name)


class FileSessionService(SessionService):
    """
    This sample session service stores session information in a temporary
    directory and identifies the session via a cookie in the request/response
    cycle. It is based on werkzeug's FilesystemSessionStore, that implements
    the whole logic for creating the actual session objects (which are
    inherited from the builtin `dict`)
    """
    def __init__(self, cookie_usage='SESSION'):
        self.cookie_usage = cookie_usage

    def _store_get(self, request):
        path = request.cfg.session_dir
        try:
            filesys.mkdir(path)
        except OSError:
            pass
        return FilesystemSessionStore(path=path, filename_template='%s',
                                      session_class=MoinSession, mode=0666 & config.umask)

    def get_session(self, request, sid=None):
        if sid is None:
            cookie_name = get_cookie_name(request, name=request.cfg.cookie_name, usage=self.cookie_usage)
            sid = request.cookies.get(cookie_name, None)
        logging.debug("get_session for sid %r" % sid)
        store = self._store_get(request)
        if sid is None:
            session = store.new()
        else:
            session = store.get(sid)
            expiry = session.get('expires')
            if expiry is not None:
                now = time.time()
                if expiry < now:
                    # the browser should've killed that cookie already.
                    # clock not in sync? trying to cheat?
                    logging.debug("session has expired (expiry: %r now: %r)" % (expiry, now))
                    self.destroy_session(request, session)
                    session = store.new()
        logging.debug("get_session returns session %r" % session)
        return session

    def get_all_session_ids(self, request):
        store = self._store_get(request)
        return store.list()

    def destroy_session(self, request, session):
        session.clear()
        store = self._store_get(request)
        store.delete(session)

    def finalize(self, request, session):
        if request.user.auth_method == 'setuid':
            userobj = request._setuid_real_user
            setuid = request.user.id
        else:
            userobj = request.user
            setuid = None
        logging.debug("finalize userobj = %r, setuid = %r" % (userobj, setuid))

        cfg = request.cfg
        # we use different cookie names for different wikis:
        cookie_name = get_cookie_name(request, name=cfg.cookie_name, usage=self.cookie_usage)
        # we always use path='/' except if explicitly overridden by configuration,
        # which is usually not needed and not recommended:
        cookie_path = cfg.cookie_path or '/'
        # a secure cookie is not transmitted over unsecure connections:
        cookie_secure = (cfg.cookie_secure or  # True means: force secure cookies
                         cfg.cookie_secure is None and request.is_secure)  # None means: https -> secure cookie

        cookie_lifetime = _get_session_lifetime(request, userobj)
        # we use 60s granularity, so we don't trigger session storage updates too often
        cookie_expires = int(time.time() / 60) * 60 + cookie_lifetime
        # when transiting logged-in -> logged out we want to kill the session
        # to protect privacy (do not show trail, even if anon sessions are on)
        kill_session = not userobj.valid and 'user.id' in session
        if kill_session:
            logging.debug("logout detected, will kill session")
        if cookie_lifetime and not kill_session:
            logging.debug("setting session cookie: %r" % (session.sid, ))
            request.set_cookie(cookie_name, session.sid,
                               max_age=cookie_lifetime, expires=cookie_expires,
                               path=cookie_path, domain=cfg.cookie_domain,
                               secure=cookie_secure, httponly=cfg.cookie_httponly)
        elif not session.new:
            # we still got a cookie, but we don't want it. kill it.
            logging.debug("deleting session cookie!")
            request.delete_cookie(cookie_name, path=cookie_path, domain=cfg.cookie_domain)

        def update_session(key, val):
            """ put key/val into session, avoid writing if it is unchanged """
            try:
                current_val = session[key]
            except KeyError:
                session[key] = val
            else:
                if val != current_val:
                    session[key] = val

        if not session.new:
            # add some info about expiry to the sessions, so we can purge them.
            # also, make sure we notice server-side if a session is expired, do
            # not rely on the client to expire the cookie.
            update_session('expires', cookie_expires)

        if cookie_lifetime and not kill_session:
            # we have set the cookie, now update the session store

            if userobj.valid:
                # we have a logged-in user
                update_session('user.id', userobj.id)
                update_session('user.auth_method', userobj.auth_method)
                update_session('user.auth_attribs', userobj.auth_attribs)
                if setuid:
                    update_session('setuid', setuid)
                elif 'setuid' in session:
                    del session['setuid']
                logging.debug("storing valid user into session: %r" % userobj.name)
            else:
                # no logged-in user (not logged in or just has logged out)
                for key in ['user.id', 'user.auth_method', 'user.auth_attribs',
                            'setuid', ]:
                    if key in session:
                        del session[key]
                logging.debug("no valid user, cleaned user info from session")

            if ((not userobj.valid and not session.new  # anon users with a cookie (not first request)
                 or
                 userobj.valid) # logged-in users, even if THIS was the first request (no cookie yet)
                                # XXX if UA doesn't support cookies, this creates 1 session file per request
                and
                session.should_save # only if we really have something (modified) to save
               ):
                store = self._store_get(request)
                logging.debug("saving session: %r" % session)
                store.save(session)
        elif not session.new:
            # we killed the cookie (see above), so we can kill the session store, too
            logging.debug("destroying session: %r" % session)
            self.destroy_session(request, session)

