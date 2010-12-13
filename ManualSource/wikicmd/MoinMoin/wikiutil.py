# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Wiki Utility Functions

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2004 by Florian Festi,
                2006 by Mikko Virkkil,
                2005-2008 MoinMoin:ThomasWaldmann,
                2007 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

import cgi
import codecs
import os
import re
import time
import urllib

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin import config
from MoinMoin.util import pysupport, lock
from MoinMoin.support.python_compatibility import rsplit
from inspect import getargspec, isfunction, isclass, ismethod

from MoinMoin import web # needed so that next line works:
import werkzeug

# Exceptions
class InvalidFileNameError(Exception):
    """ Called when we find an invalid file name """
    pass

# constants for page names
PARENT_PREFIX = "../"
PARENT_PREFIX_LEN = len(PARENT_PREFIX)
CHILD_PREFIX = "/"
CHILD_PREFIX_LEN = len(CHILD_PREFIX)

#############################################################################
### Getting data from user/Sending data to user
#############################################################################

def decodeUnknownInput(text):
    """ Decode unknown input, like text attachments

    First we try utf-8 because it has special format, and it will decode
    only utf-8 files. Then we try config.charset, then iso-8859-1 using
    'replace'. We will never raise an exception, but may return junk
    data.

    WARNING: Use this function only for data that you view, not for data
    that you save in the wiki.

    @param text: the text to decode, string
    @rtype: unicode
    @return: decoded text (maybe wrong)
    """
    # Shortcut for unicode input
    if isinstance(text, unicode):
        return text

    try:
        return unicode(text, 'utf-8')
    except UnicodeError:
        if config.charset not in ['utf-8', 'iso-8859-1']:
            try:
                return unicode(text, config.charset)
            except UnicodeError:
                pass
        return unicode(text, 'iso-8859-1', 'replace')


def decodeUserInput(s, charsets=[config.charset]):
    """
    Decodes input from the user.

    @param s: the string to unquote
    @param charsets: list of charsets to assume the string is in
    @rtype: unicode
    @return: the unquoted string as unicode
    """
    for charset in charsets:
        try:
            return s.decode(charset)
        except UnicodeError:
            pass
    raise UnicodeError('The string %r cannot be decoded.' % s)


def url_quote(s, safe='/', want_unicode=None):
    """ see werkzeug.url_quote, we use a different safe param default value """
    try:
        assert want_unicode is None
    except AssertionError:
        log.exception("call with deprecated want_unicode param, please fix caller")
    return werkzeug.url_quote(s, charset=config.charset, safe=safe)

def url_quote_plus(s, safe='/', want_unicode=None):
    """ see werkzeug.url_quote_plus, we use a different safe param default value """
    try:
        assert want_unicode is None
    except AssertionError:
        log.exception("call with deprecated want_unicode param, please fix caller")
    return werkzeug.url_quote_plus(s, charset=config.charset, safe=safe)

def url_unquote(s, want_unicode=None):
    """ see werkzeug.url_unquote """
    try:
        assert want_unicode is None
    except AssertionError:
        log.exception("call with deprecated want_unicode param, please fix caller")
    if isinstance(s, unicode):
        s = s.encode(config.charset)
    return werkzeug.url_unquote(s, charset=config.charset, errors='fallback:iso-8859-1')


def parseQueryString(qstr, want_unicode=None):
    """ see werkzeug.url_decode

        Please note: this returns a MultiDict, you might need to use dict() on
                     the result if your code expects a "normal" dict.
    """
    try:
        assert want_unicode is None
    except AssertionError:
        log.exception("call with deprecated want_unicode param, please fix caller")
    return werkzeug.url_decode(qstr, charset=config.charset, errors='fallback:iso-8859-1',
                               decode_keys=False, include_empty=False)

def makeQueryString(qstr=None, want_unicode=None, **kw):
    """ Make a querystring from arguments.

    kw arguments overide values in qstr.

    If a string is passed in, it's returned verbatim and keyword parameters are ignored.

    See also: werkzeug.url_encode

    @param qstr: dict to format as query string, using either ascii or unicode
    @param kw: same as dict when using keywords, using ascii or unicode
    @rtype: string
    @return: query string ready to use in a url
    """
    try:
        assert want_unicode is None
    except AssertionError:
        log.exception("call with deprecated want_unicode param, please fix caller")
    if qstr is None:
        qstr = {}
    elif isinstance(qstr, (str, unicode)):
        return qstr
    if isinstance(qstr, dict):
        qstr.update(kw)
        return werkzeug.url_encode(qstr, charset=config.charset, encode_keys=True)
    else:
        raise ValueError("Unsupported argument type, should be dict.")


def quoteWikinameURL(pagename, charset=config.charset):
    """ Return a url encoding of filename in plain ascii

    Use urllib.quote to quote any character that is not always safe.

    @param pagename: the original pagename (unicode)
    @param charset: url text encoding, 'utf-8' recommended. Other charset
                    might not be able to encode the page name and raise
                    UnicodeError. (default config.charset ('utf-8')).
    @rtype: string
    @return: the quoted filename, all unsafe characters encoded
    """
    # XXX please note that urllib.quote and werkzeug.url_quote have
    # XXX different defaults for safe=...
    return werkzeug.url_quote(pagename, charset=charset, safe='/')


escape = werkzeug.escape


def clean_input(text, max_len=201):
    """ Clean input:
        replace CR, LF, TAB by whitespace
        delete control chars

        @param text: unicode text to clean (if we get str, we decode)
        @rtype: unicode
        @return: cleaned text
    """
    # we only have input fields with max 200 chars, but spammers send us more
    length = len(text)
    if length == 0 or length > max_len:
        return u''
    else:
        if isinstance(text, str):
            # the translate() below can ONLY process unicode, thus, if we get
            # str, we try to decode it using the usual coding:
            text = text.decode(config.charset)
        return text.translate(config.clean_input_translation_map)


def make_breakable(text, maxlen):
    """ make a text breakable by inserting spaces into nonbreakable parts
    """
    text = text.split(" ")
    newtext = []
    for part in text:
        if len(part) > maxlen:
            while part:
                newtext.append(part[:maxlen])
                part = part[maxlen:]
        else:
            newtext.append(part)
    return " ".join(newtext)

########################################################################
### Storage
########################################################################

# Precompiled patterns for file name [un]quoting
UNSAFE = re.compile(r'[^a-zA-Z0-9_]+')
QUOTED = re.compile(r'\(([a-fA-F0-9]+)\)')


def quoteWikinameFS(wikiname, charset=config.charset):
    """ Return file system representation of a Unicode WikiName.

    Warning: will raise UnicodeError if wikiname can not be encoded using
    charset. The default value of config.charset, 'utf-8' can encode any
    character.

    @param wikiname: Unicode string possibly containing non-ascii characters
    @param charset: charset to encode string
    @rtype: string
    @return: quoted name, safe for any file system
    """
    filename = wikiname.encode(charset)

    quoted = []
    location = 0
    for needle in UNSAFE.finditer(filename):
        # append leading safe stuff
        quoted.append(filename[location:needle.start()])
        location = needle.end()
        # Quote and append unsafe stuff
        quoted.append('(')
        for character in needle.group():
            quoted.append('%02x' % ord(character))
        quoted.append(')')

    # append rest of string
    quoted.append(filename[location:])
    return ''.join(quoted)


def unquoteWikiname(filename, charsets=[config.charset]):
    """ Return Unicode WikiName from quoted file name.

    We raise an InvalidFileNameError if we find an invalid name, so the
    wiki could alarm the admin or suggest the user to rename a page.
    Invalid file names should never happen in normal use, but are rather
    cheap to find.

    This function should be used only to unquote file names, not page
    names we receive from the user. These are handled in request by
    urllib.unquote, decodePagename and normalizePagename.

    Todo: search clients of unquoteWikiname and check for exceptions.

    @param filename: string using charset and possibly quoted parts
    @param charsets: list of charsets used by string
    @rtype: Unicode String
    @return: WikiName
    """
    ### Temporary fix start ###
    # From some places we get called with Unicode strings
    if isinstance(filename, type(u'')):
        filename = filename.encode(config.charset)
    ### Temporary fix end ###

    parts = []
    start = 0
    for needle in QUOTED.finditer(filename):
        # append leading unquoted stuff
        parts.append(filename[start:needle.start()])
        start = needle.end()
        # Append quoted stuff
        group = needle.group(1)
        # Filter invalid filenames
        if (len(group) % 2 != 0):
            raise InvalidFileNameError(filename)
        try:
            for i in range(0, len(group), 2):
                byte = group[i:i+2]
                character = chr(int(byte, 16))
                parts.append(character)
        except ValueError:
            # byte not in hex, e.g 'xy'
            raise InvalidFileNameError(filename)

    # append rest of string
    if start == 0:
        wikiname = filename
    else:
        parts.append(filename[start:len(filename)])
        wikiname = ''.join(parts)

    # FIXME: This looks wrong, because at this stage "()" can be both errors
    # like open "(" without close ")", or unquoted valid characters in the file name.
    # Filter invalid filenames. Any left (xx) must be invalid
    #if '(' in wikiname or ')' in wikiname:
    #    raise InvalidFileNameError(filename)

    wikiname = decodeUserInput(wikiname, charsets)
    return wikiname

# time scaling
def timestamp2version(ts):
    """ Convert UNIX timestamp (may be float or int) to our version
        (long) int.
        We don't want to use floats, so we just scale by 1e6 to get
        an integer in usecs.
    """
    return long(ts*1000000L) # has to be long for py 2.2.x

def version2timestamp(v):
    """ Convert version number to UNIX timestamp (float).
        This must ONLY be used for display purposes.
    """
    return v / 1000000.0


# This is the list of meta attribute names to be treated as integers.
# IMPORTANT: do not use any meta attribute names with "-" (or any other chars
# invalid in python attribute names), use e.g. _ instead.
INTEGER_METAS = ['current', 'revision', # for page storage (moin 2.0)
                 'data_format_revision', # for data_dir format spec (use by mig scripts)
                ]

class MetaDict(dict):
    """ store meta informations as a dict.
    """
    def __init__(self, metafilename, cache_directory):
        """ create a MetaDict from metafilename """
        dict.__init__(self)
        self.metafilename = metafilename
        self.dirty = False
        lock_dir = os.path.join(cache_directory, '__metalock__')
        self.rlock = lock.ReadLock(lock_dir, 60.0)
        self.wlock = lock.WriteLock(lock_dir, 60.0)

        if not self.rlock.acquire(3.0):
            raise EnvironmentError("Could not lock in MetaDict")
        try:
            self._get_meta()
        finally:
            self.rlock.release()

    def _get_meta(self):
        """ get the meta dict from an arbitrary filename.
            does not keep state, does uncached, direct disk access.
            @param metafilename: the name of the file to read
            @return: dict with all values or {} if empty or error
        """

        try:
            metafile = codecs.open(self.metafilename, "r", "utf-8")
            meta = metafile.read() # this is much faster than the file's line-by-line iterator
            metafile.close()
        except IOError:
            meta = u''
        for line in meta.splitlines():
            key, value = line.split(':', 1)
            value = value.strip()
            if key in INTEGER_METAS:
                value = int(value)
            dict.__setitem__(self, key, value)

    def _put_meta(self):
        """ put the meta dict into an arbitrary filename.
            does not keep or modify state, does uncached, direct disk access.
            @param metafilename: the name of the file to write
            @param metadata: dict of the data to write to the file
        """
        meta = []
        for key, value in self.items():
            if key in INTEGER_METAS:
                value = str(value)
            meta.append("%s: %s" % (key, value))
        meta = '\r\n'.join(meta)

        metafile = codecs.open(self.metafilename, "w", "utf-8")
        metafile.write(meta)
        metafile.close()
        self.dirty = False

    def sync(self, mtime_usecs=None):
        """ No-Op except for that parameter """
        if not mtime_usecs is None:
            self.__setitem__('mtime', str(mtime_usecs))
        # otherwise no-op

    def __getitem__(self, key):
        """ We don't care for cache coherency here. """
        return dict.__getitem__(self, key)

    def __setitem__(self, key, value):
        """ Sets a dictionary entry. """
        if not self.wlock.acquire(5.0):
            raise EnvironmentError("Could not lock in MetaDict")
        try:
            self._get_meta() # refresh cache
            try:
                oldvalue = dict.__getitem__(self, key)
            except KeyError:
                oldvalue = None
            if value != oldvalue:
                dict.__setitem__(self, key, value)
                self._put_meta() # sync cache
        finally:
            self.wlock.release()


# Quoting of wiki names, file names, etc. (in the wiki markup) -----------------------------------

# don't ever change this - DEPRECATED, only needed for 1.5 > 1.6 migration conversion
QUOTE_CHARS = u'"'


#############################################################################
### InterWiki
#############################################################################
INTERWIKI_PAGE = "InterWikiMap"

def generate_file_list(request):
    """ generates a list of all files. for internal use. """

    # order is important here, the local intermap file takes
    # precedence over the shared one, and is thus read AFTER
    # the shared one
    intermap_files = request.cfg.shared_intermap
    if not isinstance(intermap_files, list):
        intermap_files = [intermap_files]
    else:
        intermap_files = intermap_files[:]
    intermap_files.append(os.path.join(request.cfg.data_dir, "intermap.txt"))
    request.cfg.shared_intermap_files = [filename for filename in intermap_files
                                         if filename and os.path.isfile(filename)]


def get_max_mtime(file_list, page):
    """ Returns the highest modification time of the files in file_list and the
    page page. """
    timestamps = [os.stat(filename).st_mtime for filename in file_list]
    if page.exists():
        # exists() is cached and thus cheaper than mtime_usecs()
        timestamps.append(version2timestamp(page.mtime_usecs()))
    if timestamps:
        return max(timestamps)
    else:
        return 0 # no files / pages there

def load_wikimap(request):
    """ load interwiki map (once, and only on demand) """
    from MoinMoin.Page import Page

    now = int(time.time())
    if getattr(request.cfg, "shared_intermap_files", None) is None:
        generate_file_list(request)

    try:
        _interwiki_list = request.cfg.cache.interwiki_list
        old_mtime = request.cfg.cache.interwiki_mtime
        if request.cfg.cache.interwiki_ts + (1*60) < now: # 1 minutes caching time
            max_mtime = get_max_mtime(request.cfg.shared_intermap_files, Page(request, INTERWIKI_PAGE))
            if max_mtime > old_mtime:
                raise AttributeError # refresh cache
            else:
                request.cfg.cache.interwiki_ts = now
    except AttributeError:
        _interwiki_list = {}
        lines = []

        for filename in request.cfg.shared_intermap_files:
            f = codecs.open(filename, "r", config.charset)
            lines.extend(f.readlines())
            f.close()

        # add the contents of the InterWikiMap page
        lines += Page(request, INTERWIKI_PAGE).get_raw_body().splitlines()

        for line in lines:
            if not line or line[0] == '#':
                continue
            try:
                line = "%s %s/InterWiki" % (line, request.script_root)
                wikitag, urlprefix, dummy = line.split(None, 2)
            except ValueError:
                pass
            else:
                _interwiki_list[wikitag] = urlprefix

        del lines

        # add own wiki as "Self" and by its configured name
        _interwiki_list['Self'] = request.script_root + '/'
        if request.cfg.interwikiname:
            _interwiki_list[request.cfg.interwikiname] = request.script_root + '/'

        # save for later
        request.cfg.cache.interwiki_list = _interwiki_list
        request.cfg.cache.interwiki_ts = now
        request.cfg.cache.interwiki_mtime = get_max_mtime(request.cfg.shared_intermap_files, Page(request, INTERWIKI_PAGE))

    return _interwiki_list

def split_wiki(wikiurl):
    """
    Split a wiki url.

    *** DEPRECATED FUNCTION FOR OLD 1.5 SYNTAX - ONLY STILL HERE FOR THE 1.5 -> 1.6 MIGRATION ***
    Use split_interwiki(), see below.

    @param wikiurl: the url to split
    @rtype: tuple
    @return: (tag, tail)
    """
    # !!! use a regex here!
    try:
        wikitag, tail = wikiurl.split(":", 1)
    except ValueError:
        try:
            wikitag, tail = wikiurl.split("/", 1)
        except ValueError:
            wikitag, tail = 'Self', wikiurl
    return wikitag, tail

def split_interwiki(wikiurl):
    """ Split a interwiki name, into wikiname and pagename, e.g:

    'MoinMoin:FrontPage' -> "MoinMoin", "FrontPage"
    'FrontPage' -> "Self", "FrontPage"
    'MoinMoin:Page with blanks' -> "MoinMoin", "Page with blanks"
    'MoinMoin:' -> "MoinMoin", ""

    can also be used for:

    'attachment:filename with blanks.txt' -> "attachment", "filename with blanks.txt"

    @param wikiurl: the url to split
    @rtype: tuple
    @return: (wikiname, pagename)
    """
    try:
        wikiname, pagename = wikiurl.split(":", 1)
    except ValueError:
        wikiname, pagename = 'Self', wikiurl
    return wikiname, pagename

def resolve_wiki(request, wikiurl):
    """
    Resolve an interwiki link.

    *** DEPRECATED FUNCTION FOR OLD 1.5 SYNTAX - ONLY STILL HERE FOR THE 1.5 -> 1.6 MIGRATION ***
    Use resolve_interwiki(), see below.

    @param request: the request object
    @param wikiurl: the InterWiki:PageName link
    @rtype: tuple
    @return: (wikitag, wikiurl, wikitail, err)
    """
    _interwiki_list = load_wikimap(request)
    # split wiki url
    wikiname, pagename = split_wiki(wikiurl)

    # return resolved url
    if wikiname in _interwiki_list:
        return (wikiname, _interwiki_list[wikiname], pagename, False)
    else:
        return (wikiname, request.script_root, "/InterWiki", True)

def resolve_interwiki(request, wikiname, pagename):
    """ Resolve an interwiki reference (wikiname:pagename).

    @param request: the request object
    @param wikiname: interwiki wiki name
    @param pagename: interwiki page name
    @rtype: tuple
    @return: (wikitag, wikiurl, wikitail, err)
    """
    _interwiki_list = load_wikimap(request)
    if wikiname in _interwiki_list:
        return (wikiname, _interwiki_list[wikiname], pagename, False)
    else:
        return (wikiname, request.script_root, "/InterWiki", True)

def join_wiki(wikiurl, wikitail):
    """
    Add a (url_quoted) page name to an interwiki url.

    Note: We can't know what kind of URL quoting a remote wiki expects.
          We just use a utf-8 encoded string with standard URL quoting.

    @param wikiurl: wiki url, maybe including a $PAGE placeholder
    @param wikitail: page name
    @rtype: string
    @return: generated URL of the page in the other wiki
    """
    wikitail = url_quote(wikitail)
    if '$PAGE' in wikiurl:
        return wikiurl.replace('$PAGE', wikitail)
    else:
        return wikiurl + wikitail


#############################################################################
### Page types (based on page names)
#############################################################################

def isSystemPage(request, pagename):
    """ Is this a system page?

    @param request: the request object
    @param pagename: the page name
    @rtype: bool
    @return: true if page is a system page
    """
    from MoinMoin import i18n
    return pagename in i18n.system_pages or isTemplatePage(request, pagename)


def isTemplatePage(request, pagename):
    """ Is this a template page?

    @param pagename: the page name
    @rtype: bool
    @return: true if page is a template page
    """
    return request.cfg.cache.page_template_regexact.search(pagename) is not None


def isGroupPage(pagename, cfg):
    """ Is this a name of group page?

    @param pagename: the page name
    @rtype: bool
    @return: true if page is a form page
    """
    return cfg.cache.page_group_regexact.search(pagename) is not None


def filterCategoryPages(request, pagelist):
    """ Return category pages in pagelist

    WARNING: DO NOT USE THIS TO FILTER THE FULL PAGE LIST! Use
    getPageList with a filter function.

    If you pass a list with a single pagename, either that is returned
    or an empty list, thus you can use this function like a `isCategoryPage`
    one.

    @param pagelist: a list of pages
    @rtype: list
    @return: only the category pages of pagelist
    """
    func = request.cfg.cache.page_category_regexact.search
    return [pn for pn in pagelist if func(pn)]


def getLocalizedPage(request, pagename): # was: getSysPage
    """ Get a system page according to user settings and available translations.

    We include some special treatment for the case that <pagename> is the
    currently rendered page, as this is the case for some pages used very
    often, like FrontPage, RecentChanges etc. - in that case we reuse the
    already existing page object instead creating a new one.

    @param request: the request object
    @param pagename: the name of the page
    @rtype: Page object
    @return: the page object of that system page, using a translated page,
             if it exists
    """
    from MoinMoin.Page import Page
    i18n_name = request.getText(pagename)
    pageobj = None
    if i18n_name != pagename:
        if request.page and i18n_name == request.page.page_name:
            # do not create new object for current page
            i18n_page = request.page
            if i18n_page.exists():
                pageobj = i18n_page
        else:
            i18n_page = Page(request, i18n_name)
            if i18n_page.exists():
                pageobj = i18n_page

    # if we failed getting a translated version of <pagename>,
    # we fall back to english
    if not pageobj:
        if request.page and pagename == request.page.page_name:
            # do not create new object for current page
            pageobj = request.page
        else:
            pageobj = Page(request, pagename)
    return pageobj


def getFrontPage(request):
    """ Convenience function to get localized front page

    @param request: current request
    @rtype: Page object
    @return localized page_front_page, if there is a translation
    """
    return getLocalizedPage(request, request.cfg.page_front_page)


def getHomePage(request, username=None):
    """
    Get a user's homepage, or return None for anon users and
    those who have not created a homepage.

    DEPRECATED - try to use getInterwikiHomePage (see below)

    @param request: the request object
    @param username: the user's name
    @rtype: Page
    @return: user's homepage object - or None
    """
    from MoinMoin.Page import Page
    # default to current user
    if username is None and request.user.valid:
        username = request.user.name

    # known user?
    if username:
        # Return home page
        page = Page(request, username)
        if page.exists():
            return page

    return None


def getInterwikiHomePage(request, username=None):
    """
    Get a user's homepage.

    cfg.user_homewiki influences behaviour of this:
    'Self' does mean we store user homepage in THIS wiki.
    When set to our own interwikiname, it behaves like with 'Self'.

    'SomeOtherWiki' means we store user homepages in another wiki.

    @param request: the request object
    @param username: the user's name
    @rtype: tuple (or None for anon users)
    @return: (wikiname, pagename)
    """
    # default to current user
    if username is None and request.user.valid:
        username = request.user.name
    if not username:
        return None # anon user

    homewiki = request.cfg.user_homewiki
    if homewiki == request.cfg.interwikiname:
        homewiki = u'Self'

    return homewiki, username


def AbsPageName(context, pagename):
    """
    Return the absolute pagename for a (possibly) relative pagename.

    @param context: name of the page where "pagename" appears on
    @param pagename: the (possibly relative) page name
    @rtype: string
    @return: the absolute page name
    """
    if pagename.startswith(PARENT_PREFIX):
        while context and pagename.startswith(PARENT_PREFIX):
            context = '/'.join(context.split('/')[:-1])
            pagename = pagename[PARENT_PREFIX_LEN:]
        pagename = '/'.join(filter(None, [context, pagename, ]))
    elif pagename.startswith(CHILD_PREFIX):
        if context:
            pagename = context + '/' + pagename[CHILD_PREFIX_LEN:]
        else:
            pagename = pagename[CHILD_PREFIX_LEN:]
    return pagename

def RelPageName(context, pagename):
    """
    Return the relative pagename for some context.

    @param context: name of the page where "pagename" appears on
    @param pagename: the absolute page name
    @rtype: string
    @return: the relative page name
    """
    if context == '':
        # special case, context is some "virtual root" page with name == ''
        # every page is a subpage of this virtual root
        return CHILD_PREFIX + pagename
    elif pagename.startswith(context + CHILD_PREFIX):
        # simple child
        return pagename[len(context):]
    else:
        # some kind of sister/aunt
        context_frags = context.split('/')   # A, B, C, D, E
        pagename_frags = pagename.split('/') # A, B, C, F
        # first throw away common parents:
        common = 0
        for cf, pf in zip(context_frags, pagename_frags):
            if cf == pf:
                common += 1
            else:
                break
        context_frags = context_frags[common:] # D, E
        pagename_frags = pagename_frags[common:] # F
        go_up = len(context_frags)
        return PARENT_PREFIX * go_up + '/'.join(pagename_frags)


def pagelinkmarkup(pagename, text=None):
    """ return markup that can be used as link to page <pagename> """
    from MoinMoin.parser.text_moin_wiki import Parser
    if re.match(Parser.word_rule + "$", pagename, re.U|re.X) and \
            (text is None or text == pagename):
        return pagename
    else:
        if text is None or text == pagename:
            text = ''
        else:
            text = '|%s' % text
        return u'[[%s%s]]' % (pagename, text)

#############################################################################
### mimetype support
#############################################################################
import mimetypes

MIMETYPES_MORE = {
 # OpenOffice 2.x & other open document stuff
 '.odt': 'application/vnd.oasis.opendocument.text',
 '.ods': 'application/vnd.oasis.opendocument.spreadsheet',
 '.odp': 'application/vnd.oasis.opendocument.presentation',
 '.odg': 'application/vnd.oasis.opendocument.graphics',
 '.odc': 'application/vnd.oasis.opendocument.chart',
 '.odf': 'application/vnd.oasis.opendocument.formula',
 '.odb': 'application/vnd.oasis.opendocument.database',
 '.odi': 'application/vnd.oasis.opendocument.image',
 '.odm': 'application/vnd.oasis.opendocument.text-master',
 '.ott': 'application/vnd.oasis.opendocument.text-template',
 '.ots': 'application/vnd.oasis.opendocument.spreadsheet-template',
 '.otp': 'application/vnd.oasis.opendocument.presentation-template',
 '.otg': 'application/vnd.oasis.opendocument.graphics-template',
 # some systems (like Mac OS X) don't have some of these:
 '.patch': 'text/x-diff',
 '.diff': 'text/x-diff',
 '.py': 'text/x-python',
 '.cfg': 'text/plain',
 '.conf': 'text/plain',
 '.irc': 'text/plain',
 '.md5': 'text/plain',
 '.csv': 'text/csv',
 '.flv': 'video/x-flv',
 '.wmv': 'video/x-ms-wmv',
 '.swf': 'application/x-shockwave-flash',
 '.moin': 'text/moin-wiki',
 '.creole': 'text/creole',
}

# add all mimetype patterns of pygments
import pygments.lexers

for name, short, patterns, mime in pygments.lexers.get_all_lexers():
    for pattern in patterns:
        if pattern.startswith('*.') and mime:
            MIMETYPES_MORE[pattern[1:]] = mime[0]

[mimetypes.add_type(mimetype, ext, True) for ext, mimetype in MIMETYPES_MORE.items()]

MIMETYPES_sanitize_mapping = {
    # this stuff is text, but got application/* for unknown reasons
    ('application', 'docbook+xml'): ('text', 'docbook'),
    ('application', 'x-latex'): ('text', 'latex'),
    ('application', 'x-tex'): ('text', 'tex'),
    ('application', 'javascript'): ('text', 'javascript'),
}

MIMETYPES_spoil_mapping = {} # inverse mapping of above
for _key, _value in MIMETYPES_sanitize_mapping.items():
    MIMETYPES_spoil_mapping[_value] = _key


class MimeType(object):
    """ represents a mimetype like text/plain """

    def __init__(self, mimestr=None, filename=None):
        self.major = self.minor = None # sanitized mime type and subtype
        self.params = {} # parameters like "charset" or others
        self.charset = None # this stays None until we know for sure!
        self.raw_mimestr = mimestr

        if mimestr:
            self.parse_mimetype(mimestr)
        elif filename:
            self.parse_filename(filename)

    def parse_filename(self, filename):
        mtype, encoding = mimetypes.guess_type(filename)
        if mtype is None:
            mtype = 'application/octet-stream'
        self.parse_mimetype(mtype)

    def parse_mimetype(self, mimestr):
        """ take a string like used in content-type and parse it into components,
            alternatively it also can process some abbreviated string like "wiki"
        """
        parameters = mimestr.split(";")
        parameters = [p.strip() for p in parameters]
        mimetype, parameters = parameters[0], parameters[1:]
        mimetype = mimetype.split('/')
        if len(mimetype) >= 2:
            major, minor = mimetype[:2] # we just ignore more than 2 parts
        else:
            major, minor = self.parse_format(mimetype[0])
        self.major = major.lower()
        self.minor = minor.lower()
        for param in parameters:
            key, value = param.split('=')
            if value[0] == '"' and value[-1] == '"': # remove quotes
                value = value[1:-1]
            self.params[key.lower()] = value
        if 'charset' in self.params:
            self.charset = self.params['charset'].lower()
        self.sanitize()

    def parse_format(self, format):
        """ maps from what we currently use on-page in a #format xxx processing
            instruction to a sanitized mimetype major, minor tuple.
            can also be user later for easier entry by the user, so he can just
            type "wiki" instead of "text/moin-wiki".
        """
        format = format.lower()
        if format in config.parser_text_mimetype:
            mimetype = 'text', format
        else:
            mapping = {
                'wiki': ('text', 'moin-wiki'),
                'irc': ('text', 'irssi'),
            }
            try:
                mimetype = mapping[format]
            except KeyError:
                mimetype = 'text', 'x-%s' % format
        return mimetype

    def sanitize(self):
        """ convert to some representation that makes sense - this is not necessarily
            conformant to /etc/mime.types or IANA listing, but if something is
            readable text, we will return some text/* mimetype, not application/*,
            because we need text/plain as fallback and not application/octet-stream.
        """
        self.major, self.minor = MIMETYPES_sanitize_mapping.get((self.major, self.minor), (self.major, self.minor))

    def spoil(self):
        """ this returns something conformant to /etc/mime.type or IANA as a string,
            kind of inverse operation of sanitize(), but doesn't change self
        """
        major, minor = MIMETYPES_spoil_mapping.get((self.major, self.minor), (self.major, self.minor))
        return self.content_type(major, minor)

    def content_type(self, major=None, minor=None, charset=None, params=None):
        """ return a string suitable for Content-Type header
        """
        major = major or self.major
        minor = minor or self.minor
        params = params or self.params or {}
        if major == 'text':
            charset = charset or self.charset or params.get('charset', config.charset)
            params['charset'] = charset
        mimestr = "%s/%s" % (major, minor)
        params = ['%s="%s"' % (key.lower(), value) for key, value in params.items()]
        params.insert(0, mimestr)
        return "; ".join(params)

    def mime_type(self):
        """ return a string major/minor only, no params """
        return "%s/%s" % (self.major, self.minor)

    def module_name(self):
        """ convert this mimetype to a string useable as python module name,
            we yield the exact module name first and then proceed to shorter
            module names (useful for falling back to them, if the more special
            module is not found) - e.g. first "text_python", next "text".
            Finally, we yield "application_octet_stream" as the most general
            mimetype we have.
            Hint: the fallback handler module for text/* should be implemented
                  in module "text" (not "text_plain")
        """
        mimetype = self.mime_type()
        modname = mimetype.replace("/", "_").replace("-", "_").replace(".", "_")
        fragments = modname.split('_')
        for length in range(len(fragments), 1, -1):
            yield "_".join(fragments[:length])
        yield self.raw_mimestr
        yield fragments[0]
        yield "application_octet_stream"


#############################################################################
### Plugins
#############################################################################

class PluginError(Exception):
    """ Base class for plugin errors """

class PluginMissingError(PluginError):
    """ Raised when a plugin is not found """

class PluginAttributeError(PluginError):
    """ Raised when plugin does not contain an attribtue """


def importPlugin(cfg, kind, name, function="execute"):
    """ Import wiki or builtin plugin

    Returns <function> attr from a plugin module <name>.
    If <function> attr is missing, raise PluginAttributeError.
    If <function> is None, return the whole module object.

    If <name> plugin can not be imported, raise PluginMissingError.

    kind may be one of 'action', 'formatter', 'macro', 'parser' or any other
    directory that exist in MoinMoin or data/plugin.

    Wiki plugins will always override builtin plugins. If you want
    specific plugin, use either importWikiPlugin or importBuiltinPlugin
    directly.

    @param cfg: wiki config instance
    @param kind: what kind of module we want to import
    @param name: the name of the module
    @param function: the function name
    @rtype: any object
    @return: "function" of module "name" of kind "kind", or None
    """
    try:
        return importWikiPlugin(cfg, kind, name, function)
    except PluginMissingError:
        return importBuiltinPlugin(kind, name, function)


def importWikiPlugin(cfg, kind, name, function="execute"):
    """ Import plugin from the wiki data directory

    See importPlugin docstring.
    """
    plugins = wikiPlugins(kind, cfg)
    modname = plugins.get(name, None)
    if modname is None:
        raise PluginMissingError()
    moduleName = '%s.%s' % (modname, name)
    return importNameFromPlugin(moduleName, function)


def importBuiltinPlugin(kind, name, function="execute"):
    """ Import builtin plugin from MoinMoin package

    See importPlugin docstring.
    """
    if not name in builtinPlugins(kind):
        raise PluginMissingError()
    moduleName = 'MoinMoin.%s.%s' % (kind, name)
    return importNameFromPlugin(moduleName, function)


def importNameFromPlugin(moduleName, name):
    """ Return <name> attr from <moduleName> module,
        raise PluginAttributeError if name does not exist.

        If name is None, return the <moduleName> module object.
    """
    if name is None:
        fromlist = []
    else:
        fromlist = [name]
    module = __import__(moduleName, globals(), {}, fromlist)
    if fromlist:
        # module has the obj for module <moduleName>
        try:
            return getattr(module, name)
        except AttributeError:
            raise PluginAttributeError
    else:
        # module now has the toplevel module of <moduleName> (see __import__ docs!)
        components = moduleName.split('.')
        for comp in components[1:]:
            module = getattr(module, comp)
        return module


def builtinPlugins(kind):
    """ Gets a list of modules in MoinMoin.'kind'

    @param kind: what kind of modules we look for
    @rtype: list
    @return: module names
    """
    modulename = "MoinMoin." + kind
    return pysupport.importName(modulename, "modules")


def wikiPlugins(kind, cfg):
    """
    Gets a dict containing the names of all plugins of @kind
    as the key and the containing module name as the value.

    @param kind: what kind of modules we look for
    @rtype: dict
    @return: plugin name to containing module name mapping
    """
    # short-cut if we've loaded the dict already
    # (or already failed to load it)
    cache = cfg._site_plugin_lists
    if kind in cache:
        result = cache[kind]
    else:
        result = {}
        for modname in cfg._plugin_modules:
            try:
                module = pysupport.importName(modname, kind)
                packagepath = os.path.dirname(module.__file__)
                plugins = pysupport.getPluginModules(packagepath)
                for p in plugins:
                    if not p in result:
                        result[p] = '%s.%s' % (modname, kind)
            except AttributeError:
                pass
        cache[kind] = result
    return result


def getPlugins(kind, cfg):
    """ Gets a list of plugin names of kind

    @param kind: what kind of modules we look for
    @rtype: list
    @return: module names
    """
    # Copy names from builtin plugins - so we dont destroy the value
    all_plugins = builtinPlugins(kind)[:]

    # Add extension plugins without duplicates
    for plugin in wikiPlugins(kind, cfg):
        if plugin not in all_plugins:
            all_plugins.append(plugin)

    return all_plugins


def searchAndImportPlugin(cfg, type, name, what=None):
    type2classname = {"parser": "Parser",
                      "formatter": "Formatter",
    }
    if what is None:
        what = type2classname[type]
    mt = MimeType(name)
    plugin = None
    for module_name in mt.module_name():
        try:
            plugin = importPlugin(cfg, type, module_name, what)
            break
        except PluginMissingError:
            pass
    else:
        raise PluginMissingError("Plugin not found! (%r %r %r)" % (type, name, what))
    return plugin


#############################################################################
### Parsers
#############################################################################

def getParserForExtension(cfg, extension):
    """
    Returns the Parser class of the parser fit to handle a file
    with the given extension. The extension should be in the same
    format as os.path.splitext returns it (i.e. with the dot).
    Returns None if no parser willing to handle is found.
    The dict of extensions is cached in the config object.

    @param cfg: the Config instance for the wiki in question
    @param extension: the filename extension including the dot
    @rtype: class, None
    @returns: the parser class or None
    """
    if not hasattr(cfg.cache, 'EXT_TO_PARSER'):
        etp, etd = {}, None
        parser_plugins = getPlugins('parser', cfg)
        # force the 'highlight' parser to be the first entry in the list
        # this makes it possible to overwrite some mapping entries later, so that
        # moin will use some "better" parser for some filename extensions
        parser_plugins.remove('highlight')
        parser_plugins = ['highlight'] + parser_plugins
        for pname in parser_plugins:
            try:
                Parser = importPlugin(cfg, 'parser', pname, 'Parser')
            except PluginMissingError:
                continue
            if hasattr(Parser, 'extensions'):
                exts = Parser.extensions
                if isinstance(exts, list):
                    for ext in exts:
                        etp[ext] = Parser
                elif str(exts) == '*':
                    etd = Parser
        cfg.cache.EXT_TO_PARSER = etp
        cfg.cache.EXT_TO_PARSER_DEFAULT = etd

    return cfg.cache.EXT_TO_PARSER.get(extension, cfg.cache.EXT_TO_PARSER_DEFAULT)


#############################################################################
### Parameter parsing
#############################################################################

class BracketError(Exception):
    pass

class BracketUnexpectedCloseError(BracketError):
    def __init__(self, bracket):
        self.bracket = bracket
        BracketError.__init__(self, "Unexpected closing bracket %s" % bracket)

class BracketMissingCloseError(BracketError):
    def __init__(self, bracket):
        self.bracket = bracket
        BracketError.__init__(self, "Missing closing bracket %s" % bracket)

class ParserPrefix:
    """
    Trivial container-class holding a single character for
    the possible prefixes for parse_quoted_separated_ext
    and implementing rich equal comparison.
    """
    def __init__(self, prefix):
        self.prefix = prefix

    def __eq__(self, other):
        return isinstance(other, ParserPrefix) and other.prefix == self.prefix

    def __repr__(self):
        return '<ParserPrefix(%s)>' % self.prefix.encode('utf-8')

def parse_quoted_separated_ext(args, separator=None, name_value_separator=None,
                               brackets=None, seplimit=0, multikey=False,
                               prefixes=None, quotes='"'):
    """
    Parses the given string according to the other parameters.

    Items can be quoted with any character from the quotes parameter
    and each quote can be escaped by doubling it, the separator and
    name_value_separator can both be quoted, when name_value_separator
    is set then the name can also be quoted.

    Values that are not given are returned as None, while the
    empty string as a value can be achieved by quoting it.

    If a name or value does not start with a quote, then the quote
    looses its special meaning for that name or value, unless it
    starts with one of the given prefixes (the parameter is unicode
    containing all allowed prefixes.) The prefixes will be returned
    as ParserPrefix() instances in the first element of the tuple
    for that particular argument.

    If multiple separators follow each other, this is treated as
    having None arguments inbetween, that is also true for when
    space is used as separators (when separator is None), filter
    them out afterwards.

    The function can also do bracketing, i.e. parse expressions
    that contain things like
        "(a (a b))" to ['(', 'a', ['(', 'a', 'b']],
    in this case, as in this example, the returned list will
    contain sub-lists and the brackets parameter must be a list
    of opening and closing brackets, e.g.
        brackets = ['()', '<>']
    Each sub-list's first item is the opening bracket used for
    grouping.
    Nesting will be observed between the different types of
    brackets given. If bracketing doesn't match, a BracketError
    instance is raised with a 'bracket' property indicating the
    type of missing or unexpected bracket, the instance will be
    either of the class BracketMissingCloseError or of the class
    BracketUnexpectedCloseError.

    If multikey is True (along with setting name_value_separator),
    then the returned tuples for (key, value) pairs can also have
    multiple keys, e.g.
        "a=b=c" -> ('a', 'b', 'c')

    @param args: arguments to parse
    @param separator: the argument separator, defaults to None, meaning any
        space separates arguments
    @param name_value_separator: separator for name=value, default '=',
        name=value keywords not parsed if evaluates to False
    @param brackets: a list of two-character strings giving
        opening and closing brackets
    @param seplimit: limits the number of parsed arguments
    @param multikey: multiple keys allowed for a single value
    @rtype: list
    @returns: list of unicode strings and tuples containing
        unicode strings, or lists containing the same for
        bracketing support
    """
    idx = 0
    assert name_value_separator is None or name_value_separator != separator
    assert name_value_separator is None or len(name_value_separator) == 1
    if not isinstance(args, unicode):
        raise TypeError('args must be unicode')
    max = len(args)
    result = []         # result list
    cur = [None]        # current item
    quoted = None       # we're inside quotes, indicates quote character used
    skipquote = 0       # next quote is a quoted quote
    noquote = False     # no quotes expected because word didn't start with one
    seplimit_reached = False # number of separators exhausted
    separator_count = 0 # number of separators encountered
    SPACE = [' ', '\t', ]
    nextitemsep = [separator]   # used for skipping trailing space
    SPACE = [' ', '\t', ]
    if separator is None:
        nextitemsep = SPACE[:]
        separators = SPACE
    else:
        nextitemsep = [separator]   # used for skipping trailing space
        separators = [separator]
    if name_value_separator:
        nextitemsep.append(name_value_separator)

    # bracketing support
    opening = []
    closing = []
    bracketstack = []
    matchingbracket = {}
    if brackets:
        for o, c in brackets:
            assert not o in opening
            opening.append(o)
            assert not c in closing
            closing.append(c)
            matchingbracket[o] = c

    def additem(result, cur, separator_count, nextitemsep):
        if len(cur) == 1:
            result.extend(cur)
        elif cur:
            result.append(tuple(cur))
        cur = [None]
        noquote = False
        separator_count += 1
        seplimit_reached = False
        if seplimit and separator_count >= seplimit:
            seplimit_reached = True
            nextitemsep = [n for n in nextitemsep if n in separators]

        return cur, noquote, separator_count, seplimit_reached, nextitemsep

    while idx < max:
        char = args[idx]
        next = None
        if idx + 1 < max:
            next = args[idx+1]
        if skipquote:
            skipquote -= 1
        if not separator is None and not quoted and char in SPACE:
            spaces = ''
            # accumulate all space
            while char in SPACE and idx < max - 1:
                spaces += char
                idx += 1
                char = args[idx]
            # remove space if args end with it
            if char in SPACE and idx == max - 1:
                break
            # remove space at end of argument
            if char in nextitemsep:
                continue
            idx -= 1
            if len(cur) and cur[-1]:
                cur[-1] = cur[-1] + spaces
        elif not quoted and char == name_value_separator:
            if multikey or len(cur) == 1:
                cur.append(None)
            else:
                if not multikey:
                    if cur[-1] is None:
                        cur[-1] = ''
                    cur[-1] += name_value_separator
                else:
                    cur.append(None)
            noquote = False
        elif not quoted and not seplimit_reached and char in separators:
            (cur, noquote, separator_count, seplimit_reached,
             nextitemsep) = additem(result, cur, separator_count, nextitemsep)
        elif not quoted and not noquote and char in quotes:
            if len(cur) and cur[-1] is None:
                del cur[-1]
            cur.append(u'')
            quoted = char
        elif char == quoted and not skipquote:
            if next == quoted:
                skipquote = 2 # will be decremented right away
            else:
                quoted = None
        elif not quoted and char in opening:
            while len(cur) and cur[-1] is None:
                del cur[-1]
            (cur, noquote, separator_count, seplimit_reached,
             nextitemsep) = additem(result, cur, separator_count, nextitemsep)
            bracketstack.append((matchingbracket[char], result))
            result = [char]
        elif not quoted and char in closing:
            while len(cur) and cur[-1] is None:
                del cur[-1]
            (cur, noquote, separator_count, seplimit_reached,
             nextitemsep) = additem(result, cur, separator_count, nextitemsep)
            cur = []
            if not bracketstack:
                raise BracketUnexpectedCloseError(char)
            expected, oldresult = bracketstack[-1]
            if not expected == char:
                raise BracketUnexpectedCloseError(char)
            del bracketstack[-1]
            oldresult.append(result)
            result = oldresult
        elif not quoted and prefixes and char in prefixes and cur == [None]:
            cur = [ParserPrefix(char)]
            cur.append(None)
        else:
            if len(cur):
                if cur[-1] is None:
                    cur[-1] = char
                else:
                    cur[-1] += char
            else:
                cur.append(char)
            noquote = True

        idx += 1

    if bracketstack:
        raise BracketMissingCloseError(bracketstack[-1][0])

    if quoted:
        if len(cur):
            if cur[-1] is None:
                cur[-1] = quoted
            else:
                cur[-1] = quoted + cur[-1]
        else:
            cur.append(quoted)

    additem(result, cur, separator_count, nextitemsep)

    return result

def parse_quoted_separated(args, separator=',', name_value=True, seplimit=0):
    result = []
    positional = result
    if name_value:
        name_value_separator = '='
        trailing = []
        keywords = {}
    else:
        name_value_separator = None

    l = parse_quoted_separated_ext(args, separator=separator,
                                   name_value_separator=name_value_separator,
                                   seplimit=seplimit)
    for item in l:
        if isinstance(item, tuple):
            key, value = item
            if key is None:
                key = u''
            keywords[key] = value
            positional = trailing
        else:
            positional.append(item)

    if name_value:
        return result, keywords, trailing
    return result

def get_bool(request, arg, name=None, default=None):
    """
    For use with values returned from parse_quoted_separated or given
    as macro parameters, return a boolean from a unicode string.
    Valid input is 'true'/'false', 'yes'/'no' and '1'/'0' or None for
    the default value.

    @param request: A request instance
    @param arg: The argument, may be None or a unicode string
    @param name: Name of the argument, for error messages
    @param default: default value if arg is None
    @rtype: boolean or None
    @returns: the boolean value of the string according to above rules
              (or default value)
    """
    _ = request.getText
    assert default is None or isinstance(default, bool)
    if arg is None:
        return default
    elif not isinstance(arg, unicode):
        raise TypeError('Argument must be None or unicode')
    arg = arg.lower()
    if arg in [u'0', u'false', u'no']:
        return False
    elif arg in [u'1', u'true', u'yes']:
        return True
    else:
        if name:
            raise ValueError(
                _('Argument "%s" must be a boolean value, not "%s"') % (
                    name, arg))
        else:
            raise ValueError(
                _('Argument must be a boolean value, not "%s"') % arg)


def get_int(request, arg, name=None, default=None):
    """
    For use with values returned from parse_quoted_separated or given
    as macro parameters, return an integer from a unicode string
    containing the decimal representation of a number.
    None is a valid input and yields the default value.

    @param request: A request instance
    @param arg: The argument, may be None or a unicode string
    @param name: Name of the argument, for error messages
    @param default: default value if arg is None
    @rtype: int or None
    @returns: the integer value of the string (or default value)
    """
    _ = request.getText
    assert default is None or isinstance(default, (int, long))
    if arg is None:
        return default
    elif not isinstance(arg, unicode):
        raise TypeError('Argument must be None or unicode')
    try:
        return int(arg)
    except ValueError:
        if name:
            raise ValueError(
                _('Argument "%s" must be an integer value, not "%s"') % (
                    name, arg))
        else:
            raise ValueError(
                _('Argument must be an integer value, not "%s"') % arg)


def get_float(request, arg, name=None, default=None):
    """
    For use with values returned from parse_quoted_separated or given
    as macro parameters, return a float from a unicode string.
    None is a valid input and yields the default value.

    @param request: A request instance
    @param arg: The argument, may be None or a unicode string
    @param name: Name of the argument, for error messages
    @param default: default return value if arg is None
    @rtype: float or None
    @returns: the float value of the string (or default value)
    """
    _ = request.getText
    assert default is None or isinstance(default, (int, long, float))
    if arg is None:
        return default
    elif not isinstance(arg, unicode):
        raise TypeError('Argument must be None or unicode')
    try:
        return float(arg)
    except ValueError:
        if name:
            raise ValueError(
                _('Argument "%s" must be a floating point value, not "%s"') % (
                    name, arg))
        else:
            raise ValueError(
                _('Argument must be a floating point value, not "%s"') % arg)


def get_complex(request, arg, name=None, default=None):
    """
    For use with values returned from parse_quoted_separated or given
    as macro parameters, return a complex from a unicode string.
    None is a valid input and yields the default value.

    @param request: A request instance
    @param arg: The argument, may be None or a unicode string
    @param name: Name of the argument, for error messages
    @param default: default return value if arg is None
    @rtype: complex or None
    @returns: the complex value of the string (or default value)
    """
    _ = request.getText
    assert default is None or isinstance(default, (int, long, float, complex))
    if arg is None:
        return default
    elif not isinstance(arg, unicode):
        raise TypeError('Argument must be None or unicode')
    try:
        # allow writing 'i' instead of 'j'
        arg = arg.replace('i', 'j').replace('I', 'j')
        return complex(arg)
    except ValueError:
        if name:
            raise ValueError(
                _('Argument "%s" must be a complex value, not "%s"') % (
                    name, arg))
        else:
            raise ValueError(
                _('Argument must be a complex value, not "%s"') % arg)


def get_unicode(request, arg, name=None, default=None):
    """
    For use with values returned from parse_quoted_separated or given
    as macro parameters, return a unicode string from a unicode string.
    None is a valid input and yields the default value.

    @param request: A request instance
    @param arg: The argument, may be None or a unicode string
    @param name: Name of the argument, for error messages
    @param default: default return value if arg is None;
    @rtype: unicode or None
    @returns: the unicode string (or default value)
    """
    assert default is None or isinstance(default, unicode)
    if arg is None:
        return default
    elif not isinstance(arg, unicode):
        raise TypeError('Argument must be None or unicode')

    return arg


def get_choice(request, arg, name=None, choices=[None], default_none=False):
    """
    For use with values returned from parse_quoted_separated or given
    as macro parameters, return a unicode string that must be in the
    choices given. None is a valid input and yields first of the valid
    choices.

    @param request: A request instance
    @param arg: The argument, may be None or a unicode string
    @param name: Name of the argument, for error messages
    @param choices: the possible choices
    @param default_none: If False (default), get_choice returns first available
                         choice if arg is None. If True, get_choice returns
                         None if arg is None. This is useful if some arg value
                         is required (no default choice).
    @rtype: unicode or None
    @returns: the unicode string (or default value)
    """
    assert isinstance(choices, (tuple, list))
    if arg is None:
        if default_none:
            return None
        else:
            return choices[0]
    elif not isinstance(arg, unicode):
        raise TypeError('Argument must be None or unicode')
    elif not arg in choices:
        _ = request.getText
        if name:
            raise ValueError(
                _('Argument "%s" must be one of "%s", not "%s"') % (
                    name, '", "'.join([repr(choice) for choice in choices]),
                    arg))
        else:
            raise ValueError(
                _('Argument must be one of "%s", not "%s"') % (
                    '", "'.join([repr(choice) for choice in choices]), arg))

    return arg


class IEFArgument:
    """
    Base class for new argument parsers for
    invoke_extension_function.
    """
    def __init__(self):
        pass

    def parse_argument(self, s):
        """
        Parse the argument given in s (a string) and return
        the argument for the extension function.
        """
        raise NotImplementedError

    def get_default(self):
        """
        Return the default for this argument.
        """
        raise NotImplementedError


class UnitArgument(IEFArgument):
    """
    Argument class for invoke_extension_function that forces
    having any of the specified units given for a value.

    Note that the default unit is "mm".

    Use, for example, "UnitArgument('7mm', float, ['%', 'mm'])".

    If the defaultunit parameter is given, any argument that
    can be converted into the given argtype is assumed to have
    the default unit. NOTE: This doesn't work with a choice
    (tuple or list) argtype.
    """
    def __init__(self, default, argtype, units=['mm'], defaultunit=None):
        """
        Initialise a UnitArgument giving the default,
        argument type and the permitted units.
        """
        IEFArgument.__init__(self)
        self._units = list(units)
        self._units.sort(lambda x, y: len(y) - len(x))
        self._type = argtype
        self._defaultunit = defaultunit
        assert defaultunit is None or defaultunit in units
        if default is not None:
            self._default = self.parse_argument(default)
        else:
            self._default = None

    def parse_argument(self, s):
        for unit in self._units:
            if s.endswith(unit):
                ret = (self._type(s[:len(s) - len(unit)]), unit)
                return ret
        if self._defaultunit is not None:
            try:
                return (self._type(s), self._defaultunit)
            except ValueError:
                pass
        units = ', '.join(self._units)
        ## XXX: how can we translate this?
        raise ValueError("Invalid unit in value %s (allowed units: %s)" % (s, units))

    def get_default(self):
        return self._default


class required_arg:
    """
    Wrap a type in this class and give it as default argument
    for a function passed to invoke_extension_function() in
    order to get generic checking that the argument is given.
    """
    def __init__(self, argtype):
        """
        Initialise a required_arg
        @param argtype: the type the argument should have
        """
        if not (argtype in (bool, int, long, float, complex, unicode) or
                isinstance(argtype, (IEFArgument, tuple, list))):
            raise TypeError("argtype must be a valid type")
        self.argtype = argtype


def invoke_extension_function(request, function, args, fixed_args=[]):
    """
    Parses arguments for an extension call and calls the extension
    function with the arguments.

    If the macro function has a default value that is a bool,
    int, long, float or unicode object, then the given value
    is converted to the type of that default value before passing
    it to the macro function. That way, macros need not call the
    wikiutil.get_* functions for any arguments that have a default.

    @param request: the request object
    @param function: the function to invoke
    @param args: unicode string with arguments (or evaluating to False)
    @param fixed_args: fixed arguments to pass as the first arguments
    @returns: the return value from the function called
    """

    def _convert_arg(request, value, default, name=None):
        """
        Using the get_* functions, convert argument to the type of the default
        if that is any of bool, int, long, float or unicode; if the default
        is the type itself then convert to that type (keeps None) or if the
        default is a list require one of the list items.

        In other cases return the value itself.
        """
        # if extending this, extend required_arg as well!
        if isinstance(default, bool):
            return get_bool(request, value, name, default)
        elif isinstance(default, (int, long)):
            return get_int(request, value, name, default)
        elif isinstance(default, float):
            return get_float(request, value, name, default)
        elif isinstance(default, complex):
            return get_complex(request, value, name, default)
        elif isinstance(default, unicode):
            return get_unicode(request, value, name, default)
        elif isinstance(default, (tuple, list)):
            return get_choice(request, value, name, default)
        elif default is bool:
            return get_bool(request, value, name)
        elif default is int or default is long:
            return get_int(request, value, name)
        elif default is float:
            return get_float(request, value, name)
        elif default is complex:
            return get_complex(request, value, name)
        elif isinstance(default, IEFArgument):
            # defaults handled later
            if value is None:
                return None
            return default.parse_argument(value)
        elif isinstance(default, required_arg):
            if isinstance(default.argtype, (tuple, list)):
                # treat choice specially and return None if no choice
                # is given in the value
                return get_choice(request, value, name, list(default.argtype),
                       default_none=True)
            else:
                return _convert_arg(request, value, default.argtype, name)
        return value

    assert isinstance(fixed_args, (list, tuple))

    _ = request.getText

    kwargs = {}
    kwargs_to_pass = {}
    trailing_args = []

    if args:
        assert isinstance(args, unicode)

        positional, keyword, trailing = parse_quoted_separated(args)

        for kw in keyword:
            try:
                kwargs[str(kw)] = keyword[kw]
            except UnicodeEncodeError:
                kwargs_to_pass[kw] = keyword[kw]

        trailing_args.extend(trailing)

    else:
        positional = []

    if isfunction(function) or ismethod(function):
        argnames, varargs, varkw, defaultlist = getargspec(function)
    elif isclass(function):
        (argnames, varargs,
         varkw, defaultlist) = getargspec(function.__init__.im_func)
    else:
        raise TypeError('function must be a function, method or class')

    # self is implicit!
    if ismethod(function) or isclass(function):
        argnames = argnames[1:]

    fixed_argc = len(fixed_args)
    argnames = argnames[fixed_argc:]
    argc = len(argnames)
    if not defaultlist:
        defaultlist = []

    # if the fixed parameters have defaults too...
    if argc < len(defaultlist):
        defaultlist = defaultlist[fixed_argc:]
    defstart = argc - len(defaultlist)

    defaults = {}
    # reverse to be able to pop() things off
    positional.reverse()
    allow_kwargs = False
    allow_trailing = False
    # convert all arguments to keyword arguments,
    # fill all arguments that weren't given with None
    for idx in range(argc):
        argname = argnames[idx]
        if argname == '_kwargs':
            allow_kwargs = True
            continue
        if argname == '_trailing_args':
            allow_trailing = True
            continue
        if positional:
            kwargs[argname] = positional.pop()
        if not argname in kwargs:
            kwargs[argname] = None
        if idx >= defstart:
            defaults[argname] = defaultlist[idx - defstart]

    if positional:
        if not allow_trailing:
            raise ValueError(_('Too many arguments'))
        trailing_args.extend(positional)

    if trailing_args:
        if not allow_trailing:
            raise ValueError(_('Cannot have arguments without name following'
                               ' named arguments'))
        kwargs['_trailing_args'] = trailing_args

    # type-convert all keyword arguments to the type
    # that the default value indicates
    for argname in kwargs.keys()[:]:
        if argname in defaults:
            # the value of 'argname' from kwargs will be put into the
            # macro's 'argname' argument, so convert that giving the
            # name to the converter so the user is told which argument
            # went wrong (if it does)
            kwargs[argname] = _convert_arg(request, kwargs[argname],
                                           defaults[argname], argname)
            if kwargs[argname] is None:
                if isinstance(defaults[argname], required_arg):
                    raise ValueError(_('Argument "%s" is required') % argname)
                if isinstance(defaults[argname], IEFArgument):
                    kwargs[argname] = defaults[argname].get_default()

        if not argname in argnames:
            # move argname into _kwargs parameter
            kwargs_to_pass[argname] = kwargs[argname]
            del kwargs[argname]

    if kwargs_to_pass:
        kwargs['_kwargs'] = kwargs_to_pass
        if not allow_kwargs:
            raise ValueError(_(u'No argument named "%s"') % (
                kwargs_to_pass.keys()[0]))

    return function(*fixed_args, **kwargs)


def parseAttributes(request, attrstring, endtoken=None, extension=None):
    """
    Parse a list of attributes and return a dict plus a possible
    error message.
    If extension is passed, it has to be a callable that returns
    a tuple (found_flag, msg). found_flag is whether it did find and process
    something, msg is '' when all was OK or any other string to return an error
    message.

    @param request: the request object
    @param attrstring: string containing the attributes to be parsed
    @param endtoken: token terminating parsing
    @param extension: extension function -
                      gets called with the current token, the parser and the dict
    @rtype: dict, msg
    @return: a dict plus a possible error message
    """
    import shlex, StringIO

    _ = request.getText

    parser = shlex.shlex(StringIO.StringIO(attrstring))
    parser.commenters = ''
    msg = None
    attrs = {}

    while not msg:
        try:
            key = parser.get_token()
        except ValueError, err:
            msg = str(err)
            break
        if not key:
            break
        if endtoken and key == endtoken:
            break

        # call extension function with the current token, the parser, and the dict
        if extension:
            found_flag, msg = extension(key, parser, attrs)
            #logging.debug("%r = extension(%r, parser, %r)" % (msg, key, attrs))
            if found_flag:
                continue
            elif msg:
                break
            #else (we found nothing, but also didn't have an error msg) we just continue below:

        try:
            eq = parser.get_token()
        except ValueError, err:
            msg = str(err)
            break
        if eq != "=":
            msg = _('Expected "=" to follow "%(token)s"') % {'token': key}
            break

        try:
            val = parser.get_token()
        except ValueError, err:
            msg = str(err)
            break
        if not val:
            msg = _('Expected a value for key "%(token)s"') % {'token': key}
            break

        key = escape(key) # make sure nobody cheats

        # safely escape and quote value
        if val[0] in ["'", '"']:
            val = escape(val)
        else:
            val = '"%s"' % escape(val, 1)

        attrs[key.lower()] = val

    return attrs, msg or ''


class ParameterParser:
    """ MoinMoin macro parameter parser

        Parses a given parameter string, separates the individual parameters
        and detects their type.

        Possible parameter types are:

        Name      | short  | example
        ----------------------------
         Integer  | i      | -374
         Float    | f      | 234.234 23.345E-23
         String   | s      | 'Stri\'ng'
         Boolean  | b      | 0 1 True false
         Name     |        | case_sensitive | converted to string

        So say you want to parse three things, name, age and if the
        person is male or not:

        The pattern will be: %(name)s%(age)i%(male)b

        As a result, the returned dict will put the first value into
        male, second into age etc. If some argument is missing, it will
        get None as its value. This also means that all the identifiers
        in the pattern will exist in the dict, they will just have the
        value None if they were not specified by the caller.

        So if we call it with the parameters as follows:
            ("John Smith", 18)
        this will result in the following dict:
            {"name": "John Smith", "age": 18, "male": None}

        Another way of calling would be:
            ("John Smith", male=True)
        this will result in the following dict:
            {"name": "John Smith", "age": None, "male": True}
    """

    def __init__(self, pattern):
        # parameter_re = "([^\"',]*(\"[^\"]*\"|'[^']*')?[^\"',]*)[,)]"
        name = "(?P<%s>[a-zA-Z_][a-zA-Z0-9_]*)"
        int_re = r"(?P<int>-?\d+)"
        bool_re = r"(?P<bool>(([10])|([Tt]rue)|([Ff]alse)))"
        float_re = r"(?P<float>-?\d+\.\d+([eE][+-]?\d+)?)"
        string_re = (r"(?P<string>('([^']|(\'))*?')|" +
                                r'("([^"]|(\"))*?"))')
        name_re = name % "name"
        name_param_re = name % "name_param"

        param_re = r"\s*(\s*%s\s*=\s*)?(%s|%s|%s|%s|%s)\s*(,|$)" % (
                   name_re, float_re, int_re, bool_re, string_re, name_param_re)
        self.param_re = re.compile(param_re, re.U)
        self._parse_pattern(pattern)

    def _parse_pattern(self, pattern):
        param_re = r"(%(?P<name>\(.*?\))?(?P<type>[ibfs]{1,3}))|\|"
        i = 0
        # TODO: Optionals aren't checked.
        self.optional = []
        named = False
        self.param_list = []
        self.param_dict = {}

        for match in re.finditer(param_re, pattern):
            if match.group() == "|":
                self.optional.append(i)
                continue
            self.param_list.append(match.group('type'))
            if match.group('name'):
                named = True
                self.param_dict[match.group('name')[1:-1]] = i
            elif named:
                raise ValueError("Named parameter expected")
            i += 1

    def __str__(self):
        return "%s, %s, optional:%s" % (self.param_list, self.param_dict,
                                        self.optional)

    def parse_parameters(self, params):
        # Default list/dict entries to None
        parameter_list = [None] * len(self.param_list)
        parameter_dict = dict([(key, None) for key in self.param_dict])
        check_list = [0] * len(self.param_list)

        i = 0
        start = 0
        fixed_count = 0
        named = False

        while start < len(params):
            match = re.match(self.param_re, params[start:])
            if not match:
                raise ValueError("malformed parameters")
            start += match.end()
            if match.group("int"):
                pvalue = int(match.group("int"))
                ptype = 'i'
            elif match.group("bool"):
                pvalue = (match.group("bool") == "1") or (match.group("bool") == "True") or (match.group("bool") == "true")
                ptype = 'b'
            elif match.group("float"):
                pvalue = float(match.group("float"))
                ptype = 'f'
            elif match.group("string"):
                pvalue = match.group("string")[1:-1]
                ptype = 's'
            elif match.group("name_param"):
                pvalue = match.group("name_param")
                ptype = 'n'
            else:
                raise ValueError("Parameter parser code does not fit param_re regex")

            name = match.group("name")
            if name:
                if name not in self.param_dict:
                    # TODO we should think on inheritance of parameters
                    raise ValueError("unknown parameter name '%s'" % name)
                nr = self.param_dict[name]
                if check_list[nr]:
                    raise ValueError("parameter '%s' specified twice" % name)
                else:
                    check_list[nr] = 1
                pvalue = self._check_type(pvalue, ptype, self.param_list[nr])
                parameter_dict[name] = pvalue
                parameter_list[nr] = pvalue
                named = True
            elif named:
                raise ValueError("only named parameters allowed after first named parameter")
            else:
                nr = i
                if nr not in self.param_dict.values():
                    fixed_count = nr + 1
                parameter_list[nr] = self._check_type(pvalue, ptype, self.param_list[nr])

            # Let's populate and map our dictionary to what's been found
            for name in self.param_dict:
                tmp = self.param_dict[name]
                parameter_dict[name] = parameter_list[tmp]

            i += 1

        for i in range(fixed_count):
            parameter_dict[i] = parameter_list[i]

        return fixed_count, parameter_dict

    def _check_type(self, pvalue, ptype, format):
        if ptype == 'n' and 's' in format: # n as s
            return pvalue

        if ptype in format:
            return pvalue # x -> x

        if ptype == 'i':
            if 'f' in format:
                return float(pvalue) # i -> f
            elif 'b' in format:
                return pvalue != 0 # i -> b
        elif ptype == 's':
            if 'b' in format:
                if pvalue.lower() == 'false':
                    return False # s-> b
                elif pvalue.lower() == 'true':
                    return True # s-> b
                else:
                    raise ValueError('%r does not match format %r' % (pvalue, format))

        if 's' in format: # * -> s
            return str(pvalue)

        raise ValueError('%r does not match format %r' % (pvalue, format))


#############################################################################
### Misc
#############################################################################
def normalize_pagename(name, cfg):
    """ Normalize page name

    Prevent creating page names with invisible characters or funny
    whitespace that might confuse the users or abuse the wiki, or
    just does not make sense.

    Restrict even more group pages, so they can be used inside acl lines.

    @param name: page name, unicode
    @rtype: unicode
    @return: decoded and sanitized page name
    """
    # Strip invalid characters
    name = config.page_invalid_chars_regex.sub(u'', name)

    # Split to pages and normalize each one
    pages = name.split(u'/')
    normalized = []
    for page in pages:
        # Ignore empty or whitespace only pages
        if not page or page.isspace():
            continue

        # Cleanup group pages.
        # Strip non alpha numeric characters, keep white space
        if isGroupPage(page, cfg):
            page = u''.join([c for c in page
                             if c.isalnum() or c.isspace()])

        # Normalize white space. Each name can contain multiple
        # words separated with only one space. Split handle all
        # 30 unicode spaces (isspace() == True)
        page = u' '.join(page.split())

        normalized.append(page)

    # Assemble components into full pagename
    name = u'/'.join(normalized)
    return name

def taintfilename(basename):
    """
    Make a filename that is supposed to be a plain name secure, i.e.
    remove any possible path components that compromise our system.

    @param basename: (possibly unsafe) filename
    @rtype: string
    @return: (safer) filename
    """
    for x in (os.pardir, ':', '/', '\\', '<', '>'):
        basename = basename.replace(x, '_')

    return basename


def drawing2fname(drawing):
    config.drawing_extensions = ['.tdraw', '.adraw',
                                 '.svg',
                                 '.png', '.jpg', '.jpeg', '.gif',
                                ]
    fname, ext = os.path.splitext(drawing)
    # note: do not just check for empty extension or stuff like drawing:foo.bar
    # will fail, instead of being expanded to foo.bar.tdraw
    if ext not in config.drawing_extensions:
        # for backwards compatibility, twikidraw is the default:
        drawing += '.tdraw'
    return drawing


def mapURL(request, url):
    """
    Map URLs according to 'cfg.url_mappings'.

    @param url: a URL
    @rtype: string
    @return: mapped URL
    """
    # check whether we have to map URLs
    if request.cfg.url_mappings:
        # check URL for the configured prefixes
        for prefix in request.cfg.url_mappings:
            if url.startswith(prefix):
                # substitute prefix with replacement value
                return request.cfg.url_mappings[prefix] + url[len(prefix):]

    # return unchanged url
    return url


def getUnicodeIndexGroup(name):
    """
    Return a group letter for `name`, which must be a unicode string.
    Currently supported: Hangul Syllables (U+AC00 - U+D7AF)

    @param name: a string
    @rtype: string
    @return: group letter or None
    """
    c = name[0]
    if u'\uAC00' <= c <= u'\uD7AF': # Hangul Syllables
        return unichr(0xac00 + (int(ord(c) - 0xac00) / 588) * 588)
    else:
        return c.upper() # we put lower and upper case words into the same index group


def isStrictWikiname(name, word_re=re.compile(ur"^(?:[%(u)s][%(l)s]+){2,}$" % {'u': config.chars_upper, 'l': config.chars_lower})):
    """
    Check whether this is NOT an extended name.

    @param name: the wikiname in question
    @rtype: bool
    @return: true if name matches the word_re
    """
    return word_re.match(name)


def is_URL(arg, schemas=config.url_schemas):
    """ Return True if arg is a URL (with a schema given in the schemas list).

        Note: there are not that many requirements for generic URLs, basically
        the only mandatory requirement is the ':' between schema and rest.
        Schema itself could be anything, also the rest (but we only support some
        schemas, as given in config.url_schemas, so it is a bit less ambiguous).
    """
    if ':' not in arg:
        return False
    for schema in schemas:
        if arg.startswith(schema + ':'):
            return True
    return False


def isPicture(url):
    """
    Is this a picture's url?

    @param url: the url in question
    @rtype: bool
    @return: true if url points to a picture
    """
    extpos = url.rfind(".") + 1
    return extpos > 1 and url[extpos:].lower() in config.browser_supported_images


def link_tag(request, params, text=None, formatter=None, on=None, **kw):
    """ Create a link.

    TODO: cleanup css_class

    @param request: the request object
    @param params: parameter string appended to the URL after the scriptname/
    @param text: text / inner part of the <a>...</a> link - does NOT get
                 escaped, so you can give HTML here and it will be used verbatim
    @param formatter: the formatter object to use
    @param on: opening/closing tag only
    @keyword attrs: additional attrs (HTMLified string) (removed in 1.5.3)
    @rtype: string
    @return: formatted link tag
    """
    if formatter is None:
        formatter = request.html_formatter
    if 'css_class' in kw:
        css_class = kw['css_class']
        del kw['css_class'] # one time is enough
    else:
        css_class = None
    id = kw.get('id', None)
    name = kw.get('name', None)
    if text is None:
        text = params # default
    if formatter:
        url = "%s/%s" % (request.script_root, params)
        # formatter.url will escape the url part
        if on is not None:
            tag = formatter.url(on, url, css_class, **kw)
        else:
            tag = (formatter.url(1, url, css_class, **kw) +
                formatter.rawHTML(text) +
                formatter.url(0))
    else: # this shouldn't be used any more:
        if on is not None and not on:
            tag = '</a>'
        else:
            attrs = ''
            if css_class:
                attrs += ' class="%s"' % css_class
            if id:
                attrs += ' id="%s"' % id
            if name:
                attrs += ' name="%s"' % name
            tag = '<a%s href="%s/%s">' % (attrs, request.script_root, params)
            if not on:
                tag = "%s%s</a>" % (tag, text)
        logging.warning("wikiutil.link_tag called without formatter and without request.html_formatter. tag=%r" % (tag, ))
    return tag

def containsConflictMarker(text):
    """ Returns true if there is a conflict marker in the text. """
    return "/!\\ '''Edit conflict" in text

def pagediff(request, pagename1, rev1, pagename2, rev2, **kw):
    """
    Calculate the "diff" between two page contents.

    @param pagename1: name of first page
    @param rev1: revision of first page
    @param pagename2: name of second page
    @param rev2: revision of second page
    @keyword ignorews: if 1: ignore pure-whitespace changes.
    @rtype: list
    @return: lines of diff output
    """
    from MoinMoin.Page import Page
    from MoinMoin.util import diff_text
    lines1 = Page(request, pagename1, rev=rev1).getlines()
    lines2 = Page(request, pagename2, rev=rev2).getlines()

    lines = diff_text.diff(lines1, lines2, **kw)
    return lines

def anchor_name_from_text(text):
    '''
    Generate an anchor name from the given text.
    This function generates valid HTML IDs matching: [A-Za-z][A-Za-z0-9:_.-]*
    Note: this transformation has a special feature: when you feed it with a
          valid ID/name, it will return it without modification (identity
          transformation).
    '''
    quoted = urllib.quote_plus(text.encode('utf-7'), safe=':')
    res = quoted.replace('%', '.').replace('+', '_')
    if not res[:1].isalpha():
        return 'A%s' % res
    return res

def split_anchor(pagename):
    """
    Split a pagename that (optionally) has an anchor into the real pagename
    and the anchor part. If there is no anchor, it returns an empty string
    for the anchor.

    Note: if pagename contains a # (as part of the pagename, not as anchor),
          you can use a trick to make it work nevertheless: just append a
          # at the end:
          "C##" returns ("C#", "")
          "Problem #1#" returns ("Problem #1", "")

    TODO: We shouldn't deal with composite pagename#anchor strings, but keep
          it separate.
          Current approach: [[pagename#anchor|label|attr=val,&qarg=qval]]
          Future approach:  [[pagename|label|attr=val,&qarg=qval,#anchor]]
          The future approach will avoid problems when there is a # in the
          pagename part (and no anchor). Also, we need to append #anchor
          at the END of the generated URL (AFTER the query string).
    """
    parts = rsplit(pagename, '#', 1)
    if len(parts) == 2:
        return parts
    else:
        return pagename, ""

########################################################################
### Tickets - usually used in forms to make sure that form submissions
### are in response to a form the same user got from moin for the same
### action and same page.
########################################################################

def createTicket(request, tm=None, action=None, pagename=None):
    """ Create a ticket using a configured secret

        @param tm: unix timestamp (optional, uses current time if not given)
        @param action: action name (optional, uses current action if not given)
                       Note: if you create a ticket for a form that calls another
                             action than the current one, you MUST specify the
                             action you call when posting the form.
        @param pagename: page name (optional, uses current page name if not given)
                       Note: if you create a ticket for a form that posts to another
                             page than the current one, you MUST specify the
                             page name you use when posting the form.
    """

    from MoinMoin.support.python_compatibility import hmac_new
    if tm is None:
        # for age-check of ticket
        tm = "%010x" % time.time()

    # make the ticket very specific:
    if pagename is None:
        try:
            pagename = request.page.page_name
        except:
            pagename = ''

    if action is None:
        action = request.action

    if request.session:
        # either a user is logged in or we have a anon session -
        # if session times out, ticket will get invalid
        sid = request.session.sid
    else:
        sid = ''

    if request.user.valid:
        uid = request.user.id
    else:
        uid = ''

    hmac_data = []
    for value in [tm, pagename, action, sid, uid, ]:
        if isinstance(value, unicode):
            value = value.encode('utf-8')
        hmac_data.append(value)

    hmac = hmac_new(request.cfg.secrets['wikiutil/tickets'],
                    ''.join(hmac_data))
    return "%s.%s" % (tm, hmac.hexdigest())


def checkTicket(request, ticket):
    """Check validity of a previously created ticket"""
    try:
        timestamp_str = ticket.split('.')[0]
        timestamp = int(timestamp_str, 16)
    except ValueError:
        # invalid or empty ticket
        logging.debug("checkTicket: invalid or empty ticket %r" % ticket)
        return False
    now = time.time()
    if timestamp < now - 10 * 3600:
        # we don't accept tickets older than 10h
        logging.debug("checkTicket: too old ticket, timestamp %r" % timestamp)
        return False
    # Note: if the session timed out, that will also invalidate the ticket,
    #       if the ticket was created within a session.
    ourticket = createTicket(request, timestamp_str)
    logging.debug("checkTicket: returning %r, got %r, expected %r" % (ticket == ourticket, ticket, ourticket))
    return ticket == ourticket


def renderText(request, Parser, text):
    """executes raw wiki markup with all page elements"""
    import StringIO
    out = StringIO.StringIO()
    request.redirect(out)
    wikiizer = Parser(text, request)
    wikiizer.format(request.formatter, inhibit_p=True)
    result = out.getvalue()
    request.redirect()
    del out
    return result

def get_processing_instructions(body):
    """ Extract the processing instructions / acl / etc. at the beginning of a page's body.

        Hint: if you have a Page object p, you already have the result of this function in
              p.meta and (even better) parsed/processed stuff in p.pi.

        Returns a list of (pi, restofline) tuples and a string with the rest of the body.
    """
    pi = []
    while body.startswith('#'):
        try:
            line, body = body.split('\n', 1) # extract first line
        except ValueError:
            line = body
            body = ''

        # end parsing on empty (invalid) PI
        if line == "#":
            body = line + '\n' + body
            break

        if line[1] == '#':# two hash marks are a comment
            comment = line[2:]
            if not comment.startswith(' '):
                # we don't require a blank after the ##, so we put one there
                comment = ' ' + comment
                line = '##%s' % comment

        verb, args = (line[1:] + ' ').split(' ', 1) # split at the first blank
        pi.append((verb.lower(), args.strip()))

    return pi, body


class Version(tuple):
    """
    Version objects store versions like 1.2.3-4.5alpha6 in a structured
    way and support version comparisons and direct version component access.
    1: major version (digits only)
    2: minor version (digits only)
    3: (maintenance) release version (digits only)
    4.5alpha6: optional additional version specification (str)

    You can create a Version instance either by giving the components, like:
        Version(1,2,3,'4.5alpha6')
    or by giving the composite version string, like:
        Version(version="1.2.3-4.5alpha6").

    Version subclasses tuple, so comparisons to tuples should work.
    Also, we inherit all the comparison logic from tuple base class.
    """
    VERSION_RE = re.compile(
        r"""(?P<major>\d+)
            \.
            (?P<minor>\d+)
            \.
            (?P<release>\d+)
            (-
             (?P<additional>.+)
            )?""",
            re.VERBOSE)

    @classmethod
    def parse_version(cls, version):
        match = cls.VERSION_RE.match(version)
        if match is None:
            raise ValueError("Unexpected version string format: %r" % version)
        v = match.groupdict()
        return int(v['major']), int(v['minor']), int(v['release']), str(v['additional'] or '')

    def __new__(cls, major=0, minor=0, release=0, additional='', version=None):
        if version:
            major, minor, release, additional = cls.parse_version(version)
        return tuple.__new__(cls, (major, minor, release, additional))

    # properties for easy access of version components
    major = property(lambda self: self[0])
    minor = property(lambda self: self[1])
    release = property(lambda self: self[2])
    additional = property(lambda self: self[3])

    def __str__(self):
        version_str = "%d.%d.%d" % (self.major, self.minor, self.release)
        if self.additional:
            version_str += "-%s" % self.additional
        return version_str

