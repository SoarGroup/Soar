# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Page class

    Page is used for read-only access to a wiki page. For r/w access see PageEditor.
    A Page object is used to access a wiki page (in general) as well as to access
    some specific revision of a wiki page.

    The RootPage is some virtual page located at / and is mainly used to do namespace
    operations like getting the page list.

    Currently, this is all a big mixture between high-level page code, intermediate
    data/underlay layering code, caching code and low-level filesystem storage code.
    To see the filesystem storage layout we use, best is to look into data/pages/
    (underlay dir uses the same format).

    TODO:
    * Cleanly separate the code into packages for:
      * Page (or rather: Item)
      * Layering
      * Cache
      * Storage
    * ACLs should be handled on a low layer, raising an Exception when access
      is denied, so we won't have security issues just because someone forgot to check
      user.may.read(secretpage).
    * The distinction between a item and a item revision should be clearer.
    * Items can be anything, not just wiki pages, but also files of any mimetype.
      The mimetype hierarchy should be modelled by a MimeTypeItem class hierarchy.

    @copyright: 2000-2004 by Juergen Hermann <jh@web.de>,
                2005-2008 by MoinMoin:ThomasWaldmann,
                2006 by MoinMoin:FlorianFesti,
                2007 by MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

import os, re, codecs

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin import config, caching, user, util, wikiutil
from MoinMoin.logfile import eventlog

def is_cache_exception(e):
    args = e.args
    return not (len(args) != 1 or args[0] != 'CacheNeedsUpdate')


class ItemCache:
    """ Cache some page item related data, as meta data or pagelist

        We only cache this to RAM in request.cfg (this is the only kind of
        server object we have), because it might be too big for pickling it
        in and out.
    """
    def __init__(self, name):
        """ Initialize ItemCache object.
            @param name: name of the object, used for display in logging and
                         influences behaviour of refresh().
        """
        self.name = name
        self.cache = {}
        self.log_pos = None # TODO: initialize this to EOF pos of log
                            # to avoid reading in the whole log on first request
        self.requests = 0
        self.hits = 0
        self.loglevel = logging.NOTSET

    def putItem(self, request, name, key, data):
        """ Remembers some data for item name under a key.
            @param request: currently unused
            @param name: name of the item (page), unicode
            @param key: used as secondary access key after name
            @param data: the data item that should be remembered
        """
        d = self.cache.setdefault(name, {})
        d[key] = data

    def getItem(self, request, name, key):
        """ Returns some item stored for item name under key.
            @param request: the request object
            @param name: name of the item (page), unicode
            @param key: used as secondary access key after name
            @return: the data or None, if there is no such name or key.
        """
        self.refresh(request)
        try:
            data = self.cache[name][key]
            self.hits += 1
            hit_str = 'hit'
        except KeyError:
            data = None
            hit_str = 'miss'
        self.requests += 1
        logging.log(self.loglevel, "%s cache %s (h/r %2.1f%%) for %r %r" % (
            self.name,
            hit_str,
            float(self.hits * 100) / self.requests,
            name,
            key,
        ))
        return data

    def refresh(self, request):
        """ Refresh the cache - if anything has changed in the wiki, we see it
            in the edit-log and either delete cached data for the changed items
            (for 'meta') or the complete cache ('pagelists').
            @param request: the request object
        """
        from MoinMoin.logfile import editlog
        elog = editlog.EditLog(request)
        old_pos = self.log_pos
        new_pos, items = elog.news(old_pos)
        if items:
            if self.name == 'meta':
                for item in items:
                    logging.log(self.loglevel, "cache: removing %r" % item)
                    try:
                        del self.cache[item]
                    except:
                        pass
            elif self.name == 'pagelists':
                logging.log(self.loglevel, "cache: clearing pagelist cache")
                self.cache = {}
        self.log_pos = new_pos # important to do this at the end -
                               # avoids threading race conditions


class Page(object):
    """ Page - Manage an (immutable) page associated with a WikiName.
        To change a page's content, use the PageEditor class.
    """
    def __init__(self, request, page_name, **kw):
        """ Create page object.

        Note that this is a 'lean' operation, since the text for the page
        is loaded on demand. Thus, things like `Page(name).link_to()` are
        efficient.

        @param page_name: WikiName of the page
        @keyword rev: number of older revision
        @keyword formatter: formatter instance or mimetype str,
                            None or no kw arg will use default formatter
        @keyword include_self: if 1, include current user (default: 0)
        """
        self.request = request
        self.cfg = request.cfg
        self.page_name = page_name
        self.rev = kw.get('rev', 0) # revision of this page
        self.include_self = kw.get('include_self', 0)

        formatter = kw.get('formatter', None)
        if isinstance(formatter, (str, unicode)): # mimetype given
            mimetype = str(formatter)
            self.formatter = None
            self.output_mimetype = mimetype
            self.default_formatter = mimetype == "text/html"
        elif formatter is not None: # formatter instance given
            self.formatter = formatter
            self.default_formatter = 0
            self.output_mimetype = "text/todo" # TODO where do we get this value from?
        else:
            self.formatter = None
            self.default_formatter = 1
            self.output_mimetype = "text/html"

        self.output_charset = config.charset # correct for wiki pages

        self._text_filename_force = None
        self.hilite_re = None

        self.__body = None # unicode page body == metadata + data
        self.__body_modified = 0 # was __body modified in RAM so it differs from disk?
        self.__meta = None # list of raw tuples of page metadata (currently: the # stuff at top of the page)
        self.__pi = None # dict of preprocessed page metadata (processing instructions)
        self.__data = None # unicode page data = body - metadata

        self.reset()

    def reset(self):
        """ Reset page state """
        page_name = self.page_name
        # page_name quoted for file system usage, needs to be reset to
        # None when pagename changes

        qpagename = wikiutil.quoteWikinameFS(page_name)
        self.page_name_fs = qpagename

        # the normal and the underlay path used for this page
        normalpath = os.path.join(self.cfg.data_dir, "pages", qpagename)
        if not self.cfg.data_underlay_dir is None:
            underlaypath = os.path.join(self.cfg.data_underlay_dir, "pages", qpagename)
        else:
            underlaypath = None

        # TUNING - remember some essential values

        # does the page come from normal page storage (0) or from
        # underlay dir (1) (can be used as index into following list)
        self._underlay = None

        # path to normal / underlay page dir
        self._pagepath = [normalpath, underlaypath]

    # now we define some properties to lazy load some attributes on first access:

    def get_body(self):
        if self.__body is None:
            # try to open file
            try:
                f = codecs.open(self._text_filename(), 'rb', config.charset)
            except IOError, er:
                import errno
                if er.errno == errno.ENOENT:
                    # just doesn't exist, return empty text (note that we
                    # never store empty pages, so this is detectable and also
                    # safe when passed to a function expecting a string)
                    return ""
                else:
                    raise

            # read file content and make sure it is closed properly
            try:
                text = f.read()
                text = self.decodeTextMimeType(text)
                self.__body = text
            finally:
                f.close()
        return self.__body

    def set_body(self, newbody):
        self.__body = newbody
        self.__meta = None
        self.__data = None
    body = property(fget=get_body, fset=set_body) # complete page text

    def get_meta(self):
        if self.__meta is None:
            self.__meta, self.__data = wikiutil.get_processing_instructions(self.body)
        return self.__meta
    meta = property(fget=get_meta) # processing instructions, ACLs (upper part of page text)

    def get_data(self):
        if self.__data is None:
            self.__meta, self.__data = wikiutil.get_processing_instructions(self.body)
        return self.__data
    data = property(fget=get_data) # content (lower part of page text)

    def get_pi(self):
        if self.__pi is None:
            self.__pi = self.parse_processing_instructions()
        return self.__pi
    pi = property(fget=get_pi) # processed meta stuff

    def getlines(self):
        """ Return a list of all lines in body.

        @rtype: list
        @return: list of strs body_lines
        """
        return self.body.split('\n')

    def get_raw_body(self):
        """ Load the raw markup from the page file.

        @rtype: unicode
        @return: raw page contents of this page, unicode
        """
        return self.body

    def get_raw_body_str(self):
        """ Returns the raw markup from the page file, as a string.

        @rtype: str
        @return: raw page contents of this page, utf-8-encoded
        """
        return self.body.encode("utf-8")

    def set_raw_body(self, body, modified=0):
        """ Set the raw body text (prevents loading from disk).

        TODO: this should not be a public function, as Page is immutable.

        @param body: raw body text
        @param modified: 1 means that we internally modified the raw text and
            that it is not in sync with the page file on disk.  This is
            used e.g. by PageEditor when previewing the page.
        """
        self.body = body
        self.__body_modified = modified

    def get_current_from_pagedir(self, pagedir):
        """ Get the current revision number from an arbitrary pagedir.
            Does not modify page object's state, uncached, direct disk access.
            @param pagedir: the pagedir with the 'current' file to read
            @return: int currentrev
        """
        revfilename = os.path.join(pagedir, 'current')
        try:
            revfile = file(revfilename)
            revstr = revfile.read().strip()
            revfile.close()
            rev = int(revstr)
        except:
            rev = 99999999 # XXX do some better error handling
        return rev

    def get_rev_dir(self, pagedir, rev=0):
        """ Get a revision of a page from an arbitrary pagedir.

        Does not modify page object's state, uncached, direct disk access.

        @param pagedir: the path to the page storage area
        @param rev: int revision to get (default is 0 and means the current
                    revision (in this case, the real revint is returned)
        @return: (str path to file of the revision,
                  int realrevint,
                  bool exists)
        """
        if rev == 0:
            rev = self.get_current_from_pagedir(pagedir)

        revstr = '%08d' % rev
        pagefile = os.path.join(pagedir, 'revisions', revstr)
        if rev != 99999999:
            exists = os.path.exists(pagefile)
            if exists:
                self._setRealPageName(pagedir)
        else:
            exists = False
        return pagefile, rev, exists

    def _setRealPageName(self, pagedir):
        """ Set page_name to the real case of page name

        On case insensitive file system, "pagename" exists even if the
        real page name is "PageName" or "PAGENAME". This leads to
        confusion in urls, links and logs.
        See MoinMoinBugs/MacHfsPlusCaseInsensitive

        Correct the case of the page name. Elements created from the
        page name in reset() are not updated because it's too messy, and
        this fix seems to be enough for now.

        Problems to fix later:

         - ["helponnavigation"] link to HelpOnNavigation but not
           considered as backlink.

        @param pagedir: the storage path to the page directory
        """
        if self._text_filename_force is None:
            # we only do this for normal pages, but not for the MissingPage,
            # because the code below is wrong in that case
            realPath = util.filesys.realPathCase(pagedir)
            if realPath is not None:
                realPath = wikiutil.unquoteWikiname(realPath)
                self.page_name = realPath[-len(self.page_name):]

    def get_rev(self, use_underlay=-1, rev=0):
        """ Get information about a revision.

        filename, number, and (existance test) of this page and revision.

        @param use_underlay: -1 == auto, 0 == normal, 1 == underlay
        @param rev: int revision to get (default is 0 and means the current
                    revision (in this case, the real revint is returned)
        @return: (str path to current revision of page,
                  int realrevint,
                  bool exists)
        """
        def layername(underlay):
            if underlay == -1:
                return 'layer_auto'
            elif underlay == 0:
                return 'layer_normal'
            else: # 1
                return 'layer_underlay'

        request = self.request
        cache_name = self.page_name
        cache_key = layername(use_underlay)
        if self._text_filename_force is None:
            cache_data = request.cfg.cache.meta.getItem(request, cache_name, cache_key)
            if cache_data and (rev == 0 or rev == cache_data[1]):
                # we got the correct rev data from the cache
                #logging.debug("got data from cache: %r %r %r" % cache_data)
                return cache_data

        # Figure out if we should use underlay or not, if needed.
        if use_underlay == -1:
            underlay, pagedir = self.getPageStatus(check_create=0)
        else:
            underlay, pagedir = use_underlay, self._pagepath[use_underlay]

        # Find current revision, if automatic selection is requested.
        if rev == 0:
            realrev = self.get_current_from_pagedir(pagedir)
        else:
            realrev = rev

        data = self.get_rev_dir(pagedir, realrev)
        if rev == 0 and self._text_filename_force is None:
            # we only save the current rev to the cache
            request.cfg.cache.meta.putItem(request, cache_name, cache_key, data)

        return data

    def current_rev(self):
        """ Return number of current revision.

        This is the same as get_rev()[1].

        @return: int revision
        """
        pagefile, rev, exists = self.get_rev()
        return rev

    def get_real_rev(self):
        """ Returns the real revision number of this page.
            A rev==0 is translated to the current revision.

        @returns: revision number > 0
        @rtype: int
        """
        if self.rev == 0:
            return self.current_rev()
        return self.rev

    def getPageBasePath(self, use_underlay=-1):
        """ Get full path to a page-specific storage area. `args` can
            contain additional path components that are added to the base path.

        @param use_underlay: force using a specific pagedir, default -1
                                -1 = automatically choose page dir
                                1 = use underlay page dir
                                0 = use standard page dir
        @rtype: string
        @return: int underlay,
                 str the full path to the storage area
        """
        standardpath, underlaypath = self._pagepath
        if underlaypath is None:
            use_underlay = 0

        if use_underlay == -1: # automatic
            if self._underlay is None:
                underlay, path = 0, standardpath
                pagefile, rev, exists = self.get_rev(use_underlay=0)
                if not exists:
                    pagefile, rev, exists = self.get_rev(use_underlay=1)
                    if exists:
                        underlay, path = 1, underlaypath
                self._underlay = underlay
            else:
                underlay = self._underlay
                path = self._pagepath[underlay]
        else: # normal or underlay
            underlay, path = use_underlay, self._pagepath[use_underlay]

        return underlay, path

    def getPageStatus(self, *args, **kw):
        """ Get full path to a page-specific storage area. `args` can
            contain additional path components that are added to the base path.

        @param args: additional path components
        @keyword use_underlay: force using a specific pagedir, default '-1'
                                -1 = automatically choose page dir
                                1 = use underlay page dir
                                0 = use standard page dir
        @keyword check_create: if true, ensures that the path requested really exists
                               (if it doesn't, create all directories automatically).
                               (default true)
        @keyword isfile: is the last component in args a filename? (default is false)
        @rtype: string
        @return: (int underlay (1 if using underlay, 0 otherwise),
                  str the full path to the storage area )
        """
        check_create = kw.get('check_create', 1)
        isfile = kw.get('isfile', 0)
        use_underlay = kw.get('use_underlay', -1)
        underlay, path = self.getPageBasePath(use_underlay)
        fullpath = os.path.join(*((path, ) + args))
        if check_create:
            if isfile:
                dirname, filename = os.path.split(fullpath)
            else:
                dirname = fullpath
            try:
                os.makedirs(dirname)
            except OSError, err:
                if not os.path.exists(dirname):
                    raise
        return underlay, fullpath

    def getPagePath(self, *args, **kw):
        """ Return path to the page storage area. """
        return self.getPageStatus(*args, **kw)[1]

    def _text_filename(self, **kw):
        """ The name of the page file, possibly of an older page.

        @keyword rev: page revision, overriding self.rev
        @rtype: string
        @return: complete filename (including path) to this page
        """
        if self._text_filename_force is not None:
            return self._text_filename_force
        rev = kw.get('rev', 0)
        if not rev and self.rev:
            rev = self.rev
        fname, rev, exists = self.get_rev(-1, rev)
        return fname

    def editlog_entry(self):
        """ Return the edit-log entry for this Page object (can be an old revision).
        """
        request = self.request
        use_cache = self.rev == 0 # use the cache for current rev
        if use_cache:
            cache_name, cache_key = self.page_name, 'lastlog'
            entry = request.cfg.cache.meta.getItem(request, cache_name, cache_key)
        else:
            entry = None
        if entry is None:
            from MoinMoin.logfile import editlog
            wanted_rev = "%08d" % self.get_real_rev()
            edit_log = editlog.EditLog(request, rootpagename=self.page_name)
            for entry in edit_log.reverse():
                if entry.rev == wanted_rev:
                    break
            else:
                entry = () # don't use None
            if use_cache:
                request.cfg.cache.meta.putItem(request, cache_name, cache_key, entry)
        return entry

    def edit_info(self):
        """ Return timestamp/editor info for this Page object (can be an old revision).

            Note: if you ask about a deleted revision, it will report timestamp and editor
                  for the delete action (in the edit-log, this is just a SAVE).

        This is used by MoinMoin/xmlrpc/__init__.py.

        @rtype: dict
        @return: timestamp and editor information
        """
        line = self.editlog_entry()
        if line:
            editordata = line.getInterwikiEditorData(self.request)
            if editordata[0] == 'interwiki':
                editor = "%s:%s" % editordata[1]
            else:
                editor = editordata[1] # ip or email or anon
            result = {
                'timestamp': line.ed_time_usecs,
                'editor': editor,
            }
            del line
        else:
            result = {}
        return result

    def last_edit(self, request):
        # XXX usage of last_edit is DEPRECATED - use edit_info()
        if not self.exists(): # XXX doesn't make much sense, but still kept
            return None       # XXX here until we remove last_edit()
        return self.edit_info()

    def lastEditInfo(self, request=None):
        """ Return the last edit info.

            Note: if you ask about a deleted revision, it will report timestamp and editor
                  for the delete action (in the edit-log, this is just a SAVE).

        @param request: the request object (DEPRECATED, unused)
        @rtype: dict
        @return: timestamp and editor information
        """
        log = self.editlog_entry()
        if log:
            request = self.request
            editor = log.getEditor(request)
            time = wikiutil.version2timestamp(log.ed_time_usecs)
            time = request.user.getFormattedDateTime(time) # Use user time format
            result = {'editor': editor, 'time': time}
            del log
        else:
            result = {}
        return result

    def isWritable(self):
        """ Can this page be changed?

        @rtype: bool
        @return: true, if this page is writable or does not exist
        """
        return os.access(self._text_filename(), os.W_OK) or not self.exists()

    def isUnderlayPage(self, includeDeleted=True):
        """ Does this page live in the underlay dir?

        Return true even if the data dir has a copy of this page. To
        check for underlay only page, use ifUnderlayPage() and not
        isStandardPage()

        @param includeDeleted: include deleted pages
        @rtype: bool
        @return: true if page lives in the underlay dir
        """
        return self.exists(domain='underlay', includeDeleted=includeDeleted)

    def isStandardPage(self, includeDeleted=True):
        """ Does this page live in the data dir?

        Return true even if this is a copy of an underlay page. To check
        for data only page, use isStandardPage() and not isUnderlayPage().

        @param includeDeleted: include deleted pages
        @rtype: bool
        @return: true if page lives in the data dir
        """
        return self.exists(domain='standard', includeDeleted=includeDeleted)

    def exists(self, rev=0, domain=None, includeDeleted=False):
        """ Does this page exist?

        This is the lower level method for checking page existence. Use
        the higher level methods isUnderlayPage and isStandardPage for
        cleaner code.

        @param rev: revision to look for. Default: check current
        @param domain: where to look for the page. Default: look in all,
                       available values: 'underlay', 'standard'
        @param includeDeleted: ignore page state, just check its pagedir
        @rtype: bool
        @return: true, if page exists
        """
        # Edge cases
        if domain == 'underlay' and not self.request.cfg.data_underlay_dir:
            return False

        if includeDeleted:
            # Look for page directory, ignore page state
            if domain is None:
                checklist = [0, 1]
            else:
                checklist = [domain == 'underlay']
            for use_underlay in checklist:
                pagedir = self.getPagePath(use_underlay=use_underlay, check_create=0)
                if os.path.exists(pagedir):
                    return True
            return False
        else:
            # Look for non-deleted pages only, using get_rev
            if not rev and self.rev:
                rev = self.rev

            if domain is None:
                use_underlay = -1
            else:
                use_underlay = domain == 'underlay'
            d, d, exists = self.get_rev(use_underlay, rev)
            return exists

    def size(self, rev=0):
        """ Get Page size.

        @rtype: int
        @return: page size, 0 for non-existent pages.
        """
        if rev == self.rev: # same revision as self
            if self.__body is not None:
                return len(self.__body)

        try:
            return os.path.getsize(self._text_filename(rev=rev))
        except EnvironmentError, e:
            import errno
            if e.errno == errno.ENOENT:
                return 0
            raise

    def mtime_usecs(self):
        """ Get modification timestamp of this page (from edit-log, can be for an old revision).

        @rtype: int
        @return: mtime of page (or 0 if page / edit-log entry does not exist)
        """
        entry = self.editlog_entry()
        return entry and entry.ed_time_usecs or 0

    def mtime_printable(self, request):
        """ Get printable (as per user's preferences) modification timestamp of this page.

        @rtype: string
        @return: formatted string with mtime of page
        """
        t = self.mtime_usecs()
        if not t:
            result = "0" # TODO: i18n, "Ever", "Beginning of time"...?
        else:
            result = request.user.getFormattedDateTime(
                wikiutil.version2timestamp(t))
        return result

    def split_title(self, force=0):
        """ Return a string with the page name split by spaces, if the user wants that.

        @param force: if != 0, then force splitting the page_name
        @rtype: unicode
        @return: pagename of this page, splitted into space separated words
        """
        request = self.request
        if not force and not request.user.wikiname_add_spaces:
            return self.page_name

        # look for the end of words and the start of a new word,
        # and insert a space there
        splitted = config.split_regex.sub(r'\1 \2', self.page_name)
        return splitted

    def url(self, request, querystr=None, anchor=None, relative=False, **kw):
        """ Return complete URL for this page, including scriptname.
            The URL is NOT escaped, if you write it to HTML, use wikiutil.escape
            (at least if you have a querystr, to escape the & chars).

        @param request: the request object
        @param querystr: the query string to add after a "?" after the url
            (str or dict, see wikiutil.makeQueryString)
        @param anchor: if specified, make a link to this anchor
        @param relative: create a relative link (default: False), note that this
                         changed in 1.7, in 1.6, the default was True.
        @rtype: str
        @return: complete url of this page, including scriptname
        """
        assert(isinstance(anchor, (type(None), str, unicode)))
        # Create url, excluding scriptname
        url = wikiutil.quoteWikinameURL(self.page_name)
        if querystr:
            if isinstance(querystr, dict):
                action = querystr.get('action', None)
            else:
                action = None # we don't support getting the action out of a str

            querystr = wikiutil.makeQueryString(querystr)

            # make action URLs denyable by robots.txt:
            if action is not None and request.cfg.url_prefix_action is not None:
                url = "%s/%s/%s" % (request.cfg.url_prefix_action, action, url)
            url = '%s?%s' % (url, querystr)

        if not relative:
            url = '%s/%s' % (request.script_root, url)

        # Add anchor
        if anchor:
            fmt = getattr(self, 'formatter', request.html_formatter)
            if fmt:
                anchor = fmt.sanitize_to_id(anchor)
            url = "%s#%s" % (url, anchor)

        return url

    def link_to_raw(self, request, text, querystr=None, anchor=None, **kw):
        """ core functionality of link_to, without the magic """
        url = self.url(request, querystr, anchor=anchor, relative=True) # scriptName is added by link_tag
        # escaping is done by link_tag -> formatter.url -> ._open()
        link = wikiutil.link_tag(request, url, text,
                                 formatter=getattr(self, 'formatter', None), **kw)
        return link

    def link_to(self, request, text=None, querystr=None, anchor=None, **kw):
        """ Return HTML markup that links to this page.

        See wikiutil.link_tag() for possible keyword parameters.

        @param request: the request object
        @param text: inner text of the link - it gets automatically escaped
        @param querystr: the query string to add after a "?" after the url
        @param anchor: if specified, make a link to this anchor
        @keyword on: opening/closing tag only
        @keyword attachment_indicator: if 1, add attachment indicator after link tag
        @keyword css_class: css class to use
        @rtype: string
        @return: formatted link
        """
        if not text:
            text = self.split_title()
        text = wikiutil.escape(text)

        # Add css class for non existing page
        if not self.exists():
            kw['css_class'] = 'nonexistent'

        attachment_indicator = kw.get('attachment_indicator')
        if attachment_indicator is None:
            attachment_indicator = 0 # default is off
        else:
            del kw['attachment_indicator'] # avoid having this as <a> tag attribute

        link = self.link_to_raw(request, text, querystr, anchor, **kw)

        # Create a link to attachments if any exist
        if attachment_indicator:
            from MoinMoin.action import AttachFile
            link += AttachFile.getIndicator(request, self.page_name)

        return link

    def getSubscribers(self, request, **kw):
        """ Get all subscribers of this page.

        @param request: the request object
        @keyword include_self: if 1, include current user (default: 0)
        @keyword return_users: if 1, return user instances (default: 0)
        @rtype: dict
        @return: lists of subscribed email addresses in a dict by language key
        """
        include_self = kw.get('include_self', self.include_self)
        return_users = kw.get('return_users', 0)

        # extract categories of this page
        pageList = self.getCategories(request)

        # add current page name for list matching
        pageList.append(self.page_name)

        if self.cfg.SecurityPolicy:
            UserPerms = self.cfg.SecurityPolicy
        else:
            from MoinMoin.security import Default as UserPerms

        # get email addresses of the all wiki user which have a profile stored;
        # add the address only if the user has subscribed to the page and
        # the user is not the current editor
        userlist = user.getUserList(request)
        subscriber_list = {}
        for uid in userlist:
            if uid == request.user.id and not include_self:
                continue # no self notification
            subscriber = user.User(request, uid)

            # The following tests should be ordered in order of
            # decreasing computation complexity, in particular
            # the permissions check may be expensive; see the bug
            # MoinMoinBugs/GetSubscribersPerformanceProblem

            # This is a bit wrong if return_users=1 (which implies that the caller will process
            # user attributes and may, for example choose to send an SMS)
            # So it _should_ be "not (subscriber.email and return_users)" but that breaks at the moment.
            if not subscriber.email:
                continue # skip empty email addresses

            # skip people not subscribed
            if not subscriber.isSubscribedTo(pageList):
                continue

            # skip people who can't read the page
            if not UserPerms(subscriber).read(self.page_name):
                continue

            # add the user to the list
            lang = subscriber.language or request.cfg.language_default
            if not lang in subscriber_list:
                subscriber_list[lang] = []
            if return_users:
                subscriber_list[lang].append(subscriber)
            else:
                subscriber_list[lang].append(subscriber.email)

        return subscriber_list

    def parse_processing_instructions(self):
        """ Parse page text and extract processing instructions,
            return a dict of PIs and the non-PI rest of the body.
        """
        from MoinMoin import i18n
        from MoinMoin import security
        request = self.request
        pi = {} # we collect the processing instructions here

        # default language from cfg
        pi['language'] = self.cfg.language_default or "en"

        body = self.body
        # TODO: remove this hack once we have separate metadata and can use mimetype there
        if body.startswith('<?xml'): # check for XML content
            pi['lines'] = 0
            pi['format'] = "xslt"
            pi['formatargs'] = ''
            pi['acl'] = security.AccessControlList(request.cfg, []) # avoid KeyError on acl check
            return pi

        meta = self.meta

        # default is wiki markup
        pi['format'] = self.cfg.default_markup or "wiki"
        pi['formatargs'] = ''
        pi['lines'] = len(meta)
        acl = []

        for verb, args in meta:
            if verb == "format": # markup format
                format, formatargs = (args + ' ').split(' ', 1)
                pi['format'] = format.lower()
                pi['formatargs'] = formatargs.strip()

            elif verb == "acl":
                acl.append(args)

            elif verb == "language":
                # Page language. Check if args is a known moin language
                if args in i18n.wikiLanguages():
                    pi['language'] = args

            elif verb == "refresh":
                if self.cfg.refresh:
                    try:
                        mindelay, targetallowed = self.cfg.refresh
                        args = args.split()
                        if len(args) >= 1:
                            delay = max(int(args[0]), mindelay)
                        if len(args) >= 2:
                            target = args[1]
                        else:
                            target = self.page_name
                        if '://' in target:
                            if targetallowed == 'internal':
                                raise ValueError
                            elif targetallowed == 'external':
                                url = target
                        else:
                            url = Page(request, target).url(request)
                        pi['refresh'] = (delay, url)
                    except (ValueError, ):
                        pass

            elif verb == "redirect":
                pi['redirect'] = args

            elif verb == "deprecated":
                pi['deprecated'] = True

            elif verb == "openiduser":
                if request.cfg.openid_server_enable_user:
                    pi['openid.user'] = args

            elif verb == "pragma":
                try:
                    key, val = args.split(' ', 1)
                except (ValueError, TypeError):
                    pass
                else:
                    request.setPragma(key, val)

        pi['acl'] = security.AccessControlList(request.cfg, acl)
        return pi

    def send_raw(self, content_disposition=None, mimetype=None):
        """ Output the raw page data (action=raw).
            With no content_disposition, the browser usually just displays the
            data on the screen, with content_disposition='attachment', it will
            offer a dialogue to save it to disk (used by Save action).
            Supplied mimetype overrides default text/plain.
        """
        request = self.request
        request.mimetype = mimetype or 'text/plain'
        if self.exists():
            # use the correct last-modified value from the on-disk file
            # to ensure cacheability where supported. Because we are sending
            # RAW (file) content, the file mtime is correct as Last-Modified header.
            request.status_code = 200
            request.last_modified = os.path.getmtime(self._text_filename())
            text = self.encodeTextMimeType(self.body)
            #request.headers['Content-Length'] = len(text)  # XXX WRONG! text is unicode obj, but we send utf-8!
            if content_disposition:
                # TODO: fix the encoding here, plain 8 bit is not allowed according to the RFCs
                # There is no solution that is compatible to IE except stripping non-ascii chars
                filename_enc = "%s.txt" % self.page_name.encode(config.charset)
                dispo_string = '%s; filename="%s"' % (content_disposition, filename_enc)
                request.headers['Content-Disposition'] = dispo_string
        else:
            request.status_code = 404
            text = u"Page %s not found." % self.page_name

        request.write(text)

    def send_page(self, **keywords):
        """ Output the formatted page.

        TODO: "kill send_page(), quick" (since 2002 :)

        @keyword content_only: if 1, omit http headers, page header and footer
        @keyword content_id: set the id of the enclosing div
        @keyword count_hit: if 1, add an event to the log
        @keyword send_special: if True, this is a special page send
        @keyword omit_footnotes: if True, do not send footnotes (used by include macro)
        """
        request = self.request
        _ = request.getText
        request.clock.start('send_page')
        emit_headers = keywords.get('emit_headers', 1)
        content_only = keywords.get('content_only', 0)
        omit_footnotes = keywords.get('omit_footnotes', 0)
        content_id = keywords.get('content_id', 'content')
        do_cache = keywords.get('do_cache', 1)
        send_special = keywords.get('send_special', False)
        print_mode = keywords.get('print_mode', 0)
        if print_mode:
            media = request.values.get('media', 'print')
        else:
            media = 'screen'
        self.hilite_re = (keywords.get('hilite_re') or
                          request.values.get('highlight'))

        # count hit?
        if keywords.get('count_hit', 0):
            eventlog.EventLog(request).add(request, 'VIEWPAGE', {'pagename': self.page_name})

        # load the text
        body = self.data
        pi = self.pi

        if 'redirect' in pi and not (
            'action' in request.values or 'redirect' in request.values or content_only):
            # redirect to another page
            # note that by including "action=show", we prevent endless looping
            # (see code in "request") or any cascaded redirection
            pagename, anchor = wikiutil.split_anchor(pi['redirect'])
            redirect_url = Page(request, pagename).url(request,
                                                       querystr={'action': 'show', 'redirect': self.page_name, },
                                                       anchor=anchor)
            request.http_redirect(redirect_url, code=301)
            return

        # if necessary, load the formatter
        if self.default_formatter:
            from MoinMoin.formatter.text_html import Formatter
            self.formatter = Formatter(request, store_pagelinks=1)
        elif not self.formatter:
            Formatter = wikiutil.searchAndImportPlugin(request.cfg, "formatter", self.output_mimetype)
            self.formatter = Formatter(request)

        # save formatter
        no_formatter = object()
        old_formatter = getattr(request, "formatter", no_formatter)
        request.formatter = self.formatter

        self.formatter.setPage(self)
        if self.hilite_re:
            try:
                self.formatter.set_highlight_re(self.hilite_re)
            except re.error, err:
                request.theme.add_msg(_('Invalid highlighting regular expression "%(regex)s": %(error)s') % {
                                          'regex': wikiutil.escape(self.hilite_re),
                                          'error': wikiutil.escape(str(err)),
                                      }, "warning")
                self.hilite_re = None

        if 'deprecated' in pi:
            # deprecated page, append last backup version to current contents
            # (which should be a short reason why the page is deprecated)
            request.theme.add_msg(_('The backed up content of this page is deprecated and will rank lower in search results!'), "warning")

            revisions = self.getRevList()
            if len(revisions) >= 2: # XXX shouldn't that be ever the case!? Looks like not.
                oldpage = Page(request, self.page_name, rev=revisions[1])
                body += oldpage.get_data()
                del oldpage

        lang = self.pi.get('language', request.cfg.language_default)
        request.setContentLanguage(lang)

        # start document output
        page_exists = self.exists()
        if not content_only:
            if emit_headers:
                request.content_type = "%s; charset=%s" % (self.output_mimetype, self.output_charset)
                if page_exists:
                    if not request.user.may.read(self.page_name):
                        request.status_code = 403
                    else:
                        request.status_code = 200
                    if not request.cacheable:
                        # use "nocache" headers if we're using a method that is not simply "display"
                        request.disableHttpCaching(level=2)
                    elif request.user.valid:
                        # use nocache headers if a user is logged in (which triggers personalisation features)
                        request.disableHttpCaching(level=1)
                    else:
                        # TODO: we need to know if a page generates dynamic content -
                        # if it does, we must not use the page file mtime as last modified value
                        # The following code is commented because it is incorrect for dynamic pages:
                        #lastmod = os.path.getmtime(self._text_filename())
                        #request.headers['Last-Modified'] = util.timefuncs.formathttpdate(lastmod)
                        pass
                else:
                    request.status_code = 404

            if not page_exists and self.request.isSpiderAgent:
                # don't send any 404 content to bots
                return

            request.write(self.formatter.startDocument(self.page_name))

            # send the page header
            if self.default_formatter:
                if self.rev:
                    request.theme.add_msg("<strong>%s</strong><br>" % (
                        _('Revision %(rev)d as of %(date)s') % {
                            'rev': self.rev,
                            'date': wikiutil.escape(self.mtime_printable(request))
                        }), "info")

                # This redirect message is very annoying.
                # Less annoying now without the warning sign.
                if 'redirect' in request.values:
                    redir = request.values['redirect']
                    request.theme.add_msg('<strong>%s</strong><br>' % (
                        _('Redirected from page "%(page)s"') % {'page':
                            wikiutil.link_tag(request, wikiutil.quoteWikinameURL(redir) + "?action=show", self.formatter.text(redir))}), "info")
                if 'redirect' in pi:
                    request.theme.add_msg('<strong>%s</strong><br>' % (
                        _('This page redirects to page "%(page)s"') % {'page': wikiutil.escape(pi['redirect'])}), "info")

                # Page trail
                trail = None
                if not print_mode:
                    request.user.addTrail(self)
                    trail = request.user.getTrail()

                title = self.split_title()

                html_head = ''
                if request.cfg.openid_server_enabled:
                    openid_username = self.page_name
                    userid = user.getUserId(request, openid_username)

                    if userid is None and 'openid.user' in self.pi:
                        openid_username = self.pi['openid.user']
                        userid = user.getUserId(request, openid_username)

                    openid_group_name = request.cfg.openid_server_restricted_users_group
                    if userid is not None and (
                        not openid_group_name or (
                            openid_group_name in request.groups and
                            openid_username in request.groups[openid_group_name])):
                        html_head = '<link rel="openid2.provider" href="%s">' % \
                                        wikiutil.escape(request.getQualifiedURL(self.url(request,
                                                                                querystr={'action': 'serveopenid'})), True)
                        html_head += '<link rel="openid.server" href="%s">' % \
                                        wikiutil.escape(request.getQualifiedURL(self.url(request,
                                                                                querystr={'action': 'serveopenid'})), True)
                        html_head += '<meta http-equiv="x-xrds-location" content="%s">' % \
                                        wikiutil.escape(request.getQualifiedURL(self.url(request,
                                                                                querystr={'action': 'serveopenid', 'yadis': 'ep'})), True)
                    elif self.page_name == request.cfg.page_front_page:
                        html_head = '<meta http-equiv="x-xrds-location" content="%s">' % \
                                        wikiutil.escape(request.getQualifiedURL(self.url(request,
                                                                                querystr={'action': 'serveopenid', 'yadis': 'idp'})), True)

                request.theme.send_title(title, page=self,
                                    print_mode=print_mode,
                                    media=media, pi_refresh=pi.get('refresh'),
                                    allow_doubleclick=1, trail=trail,
                                    html_head=html_head,
                                    )

        # special pages handling, including denying access
        special = None

        if not send_special:
            if not page_exists and not body:
                special = 'missing'
            elif not request.user.may.read(self.page_name):
                special = 'denied'

            # if we have a special page, output it, unless
            #  - we should only output content (this is for say the pagelinks formatter)
            #  - we have a non-default formatter
            if special and not content_only and self.default_formatter:
                self._specialPageText(request, special) # this recursively calls send_page

        # if we didn't short-cut to a special page, output this page
        if not special:
            # start wiki content div
            request.write(self.formatter.startContent(content_id))

            # parse the text and send the page content
            self.send_page_content(request, body,
                                   format=pi['format'],
                                   format_args=pi['formatargs'],
                                   do_cache=do_cache,
                                   start_line=pi['lines'])

            # check for pending footnotes
            if getattr(request, 'footnotes', None) and not omit_footnotes:
                from MoinMoin.macro.FootNote import emit_footnotes
                request.write(emit_footnotes(request, self.formatter))

            # end wiki content div
            request.write(self.formatter.endContent())

        # end document output
        if not content_only:
            # send the page footer
            if self.default_formatter:
                request.theme.send_footer(self.page_name, print_mode=print_mode)

            request.write(self.formatter.endDocument())

        request.clock.stop('send_page')
        if not content_only and self.default_formatter:
            request.theme.send_closing_html()

        # cache the pagelinks
        if do_cache and self.default_formatter and page_exists:
            cache = caching.CacheEntry(request, self, 'pagelinks', scope='item', use_pickle=True)
            if cache.needsUpdate(self._text_filename()):
                links = self.formatter.pagelinks
                cache.update(links)

        # restore old formatter (hopefully we dont throw any exception that is catched again)
        if old_formatter is no_formatter:
            del request.formatter
        else:
            request.formatter = old_formatter


    def getFormatterName(self):
        """ Return a formatter name as used in the caching system

        @rtype: string
        @return: formatter name as used in caching
        """
        if not hasattr(self, 'formatter') or self.formatter is None:
            return ''
        module = self.formatter.__module__
        return module[module.rfind('.') + 1:]

    def canUseCache(self, parser=None):
        """ Is caching available for this request?

        This make sure we can try to use the caching system for this
        request, but it does not make sure that this will
        succeed. Themes can use this to decide if a Refresh action
        should be displayed.

        @param parser: the parser used to render the page
        @rtype: bool
        @return: if this page can use caching
        """
        if (not self.rev and
            not self.hilite_re and
            not self.__body_modified and
            self.getFormatterName() in self.cfg.caching_formats):
            # Everything is fine, now check the parser:
            if parser is None:
                try:
                    parser = wikiutil.searchAndImportPlugin(self.request.cfg, "parser", self.pi['format'])
                except wikiutil.PluginMissingError:
                    return False
            return getattr(parser, 'caching', False)
        return False

    def send_page_content(self, request, body, format='wiki', format_args='', do_cache=1, **kw):
        """ Output the formatted wiki page, using caching if possible

        @param request: the request object
        @param body: text of the wiki page
        @param format: format of content, default 'wiki'
        @param format_args: #format arguments, used by some parsers
        @param do_cache: if True, use cached content
        """
        request.clock.start('send_page_content')
        # Load the parser
        try:
            Parser = wikiutil.searchAndImportPlugin(request.cfg, "parser", format)
        except wikiutil.PluginMissingError:
            Parser = wikiutil.searchAndImportPlugin(request.cfg, "parser", "plain")
        parser = Parser(body, request, format_args=format_args, **kw)

        if not (do_cache and self.canUseCache(Parser)):
            self.format(parser)
        else:
            try:
                code = self.loadCache(request)
                self.execute(request, parser, code)
            except Exception, e:
                if not is_cache_exception(e):
                    raise
                try:
                    code = self.makeCache(request, parser)
                    self.execute(request, parser, code)
                except Exception, e:
                    if not is_cache_exception(e):
                        raise
                    logging.error('page cache failed after creation')
                    self.format(parser)

        request.clock.stop('send_page_content')

    def format(self, parser):
        """ Format and write page content without caching """
        parser.format(self.formatter)

    def execute(self, request, parser, code):
        """ Write page content by executing cache code """
        formatter = self.formatter
        request.clock.start("Page.execute")
        try:
            from MoinMoin.macro import Macro
            macro_obj = Macro(parser)
            # Fix __file__ when running from a zip package
            import MoinMoin
            if hasattr(MoinMoin, '__loader__'):
                __file__ = os.path.join(MoinMoin.__loader__.archive, 'dummy')
            try:
                exec code
            except "CacheNeedsUpdate": # convert the exception
                raise Exception("CacheNeedsUpdate")
        finally:
            request.clock.stop("Page.execute")

    def loadCache(self, request):
        """ Return page content cache or raises 'CacheNeedsUpdate' """
        cache = caching.CacheEntry(request, self, self.getFormatterName(), scope='item')
        attachmentsPath = self.getPagePath('attachments', check_create=0)
        if cache.needsUpdate(self._text_filename(), attachmentsPath):
            raise Exception('CacheNeedsUpdate')

        import marshal
        try:
            return marshal.loads(cache.content())
        except (EOFError, ValueError, TypeError):
            # Bad marshal data, must update the cache.
            # See http://docs.python.org/lib/module-marshal.html
            raise Exception('CacheNeedsUpdate')
        except Exception, err:
            logging.info('failed to load "%s" cache: %s' %
                        (self.page_name, str(err)))
            raise Exception('CacheNeedsUpdate')

    def makeCache(self, request, parser):
        """ Format content into code, update cache and return code """
        import marshal
        from MoinMoin.formatter.text_python import Formatter
        formatter = Formatter(request, ["page"], self.formatter)

        # Save request state while formatting page
        saved_current_lang = request.current_lang
        try:
            text = request.redirectedOutput(parser.format, formatter)
        finally:
            request.current_lang = saved_current_lang

        src = formatter.assemble_code(text)
        code = compile(src.encode(config.charset),
                       self.page_name.encode(config.charset), 'exec')
        cache = caching.CacheEntry(request, self, self.getFormatterName(), scope='item')
        cache.update(marshal.dumps(code))
        return code

    def _specialPageText(self, request, special_type):
        """ Output the default page content for new pages.

        @param request: the request object
        """
        _ = request.getText

        if special_type == 'missing':
            if request.user.valid and request.user.name == self.page_name and \
               request.cfg.user_homewiki in ('Self', request.cfg.interwikiname):
                page = wikiutil.getLocalizedPage(request, 'MissingHomePage')
            else:
                page = wikiutil.getLocalizedPage(request, 'MissingPage')

            alternative_text = u"'''<<Action(action=edit, text=\"%s\")>>'''" % _('Create New Page')
        elif special_type == 'denied':
            page = wikiutil.getLocalizedPage(request, 'PermissionDeniedPage')
            alternative_text = u"'''%s'''" % _('You are not allowed to view this page.')
        else:
            assert False

        special_exists = page.exists()

        if special_exists:
            page._text_filename_force = page._text_filename()
        else:
            page.body = alternative_text
            logging.warn('The page "%s" could not be found. Check your'
                         ' underlay directory setting.' % page.page_name)
        page.page_name = self.page_name

        page.send_page(content_only=True, do_cache=not special_exists, send_special=True)


    def getRevList(self):
        """ Get a page revision list of this page, including the current version,
        sorted by revision number in descending order (current page first).

        @rtype: list of ints
        @return: page revisions
        """
        revisions = []
        if self.page_name:
            rev_dir = self.getPagePath('revisions', check_create=0)
            if os.path.isdir(rev_dir):
                for rev in os.listdir(rev_dir):
                    try:
                        revint = int(rev)
                        revisions.append(revint)
                    except ValueError:
                        pass
                revisions.sort()
                revisions.reverse()
        return revisions

    def olderrevision(self, rev=0):
        """ Get revision of the next older page revision than rev.
        rev == 0 means this page objects revision (that may be an old
        revision already!)
        """
        if rev == 0:
            rev = self.rev
        revisions = self.getRevList()
        for r in revisions:
            if r < rev:
                older = r
                break
        return older

    def getPageText(self, start=0, length=None):
        """ Convenience function to get the page text, skipping the header

        @rtype: unicode
        @return: page text, excluding the header
        """
        if length is None:
            return self.data[start:]
        else:
            return self.data[start:start+length]

    def getPageHeader(self, start=0, length=None):
        """ Convenience function to get the page header

        @rtype: unicode
        @return: page header
        """
        header = ['#%s %s' % t for t in self.meta]
        header = '\n'.join(header)
        if header:
            if length is None:
                return header[start:]
            else:
                return header[start:start+length]
        return ''

    def getPageLinks(self, request):
        """ Get a list of the links on this page.

        @param request: the request object
        @rtype: list
        @return: page names this page links to
        """
        if self.exists():
            cache = caching.CacheEntry(request, self, 'pagelinks', scope='item', do_locking=False, use_pickle=True)
            if cache.needsUpdate(self._text_filename()):
                links = self.parsePageLinks(request)
                cache.update(links)
            else:
                try:
                    links = cache.content()
                except caching.CacheError:
                    links = self.parsePageLinks(request)
                    cache.update(links)
        else:
            links = []
        return links

    def parsePageLinks(self, request):
        """ Parse page links by formatting with a pagelinks formatter

        This is a old hack to get the pagelinks by rendering the page
        with send_page. We can remove this hack after factoring
        send_page and send_page_content into small reuseable methods.

        More efficient now by using special pagelinks formatter and
        redirecting possible output into null file.
        """
        pagename = self.page_name
        if request.parsePageLinks_running.get(pagename, False):
            #logging.debug("avoid recursion for page %r" % pagename)
            return [] # avoid recursion

        #logging.debug("running parsePageLinks for page %r" % pagename)
        # remember we are already running this function for this page:
        request.parsePageLinks_running[pagename] = True

        request.clock.start('parsePageLinks')

        class Null:
            def write(self, data):
                pass

        request.redirect(Null())
        request.mode_getpagelinks += 1
        #logging.debug("mode_getpagelinks == %r" % request.mode_getpagelinks)
        try:
            try:
                from MoinMoin.formatter.pagelinks import Formatter
                formatter = Formatter(request, store_pagelinks=1)
                page = Page(request, pagename, formatter=formatter)
                page.send_page(content_only=1)
            except:
                logging.exception("pagelinks formatter failed, traceback follows")
        finally:
            request.mode_getpagelinks -= 1
            #logging.debug("mode_getpagelinks == %r" % request.mode_getpagelinks)
            request.redirect()
            if hasattr(request, '_fmt_hd_counters'):
                del request._fmt_hd_counters
            request.clock.stop('parsePageLinks')
        return formatter.pagelinks

    def getCategories(self, request):
        """ Get categories this page belongs to.

        @param request: the request object
        @rtype: list
        @return: categories this page belongs to
        """
        return wikiutil.filterCategoryPages(request, self.getPageLinks(request))

    def getParentPage(self):
        """ Return parent page or None

        @rtype: Page
        @return: parent page or None
        """
        if self.page_name:
            pos = self.page_name.rfind('/')
            if pos > 0:
                parent = Page(self.request, self.page_name[:pos])
                if parent.exists():
                    return parent
        return None

    def getACL(self, request):
        """ Get cached ACLs of this page.

        Return cached ACL or invoke parseACL and update the cache.

        @param request: the request object
        @rtype: MoinMoin.security.AccessControlList
        @return: ACL of this page
        """
        try:
            return self.__acl # for request.page, this is n-1 times used
        except AttributeError:
            # the caching here is still useful for pages != request.page,
            # when we have multiple page objects for the same page name.
            request.clock.start('getACL')
            # Try the cache or parse acl and update the cache
            currentRevision = self.current_rev()
            cache_name = self.page_name
            cache_key = 'acl'
            cache_data = request.cfg.cache.meta.getItem(request, cache_name, cache_key)
            if cache_data is None:
                aclRevision, acl = None, None
            else:
                aclRevision, acl = cache_data
            #logging.debug("currrev: %r, cachedaclrev: %r" % (currentRevision, aclRevision))
            if aclRevision != currentRevision:
                acl = self.parseACL()
                if currentRevision != 99999999:
                    # don't use cache for non existing pages
                    # otherwise in the process of creating copies by filesys.copytree (PageEditor.copyPage)
                    # the first may test will create a cache entry with the default_acls for a non existing page
                    # At the time the page is created acls on that page would be ignored until the process
                    # is completed by adding a log entry into edit-log
                    cache_data = (currentRevision, acl)
                    request.cfg.cache.meta.putItem(request, cache_name, cache_key, cache_data)
            self.__acl = acl
            request.clock.stop('getACL')
            return acl

    def parseACL(self):
        """ Return ACLs parsed from the last available revision

        The effective ACL is always from the last revision, even if
        you access an older revision.
        """
        from MoinMoin import security
        if self.exists() and self.rev == 0:
            return self.pi['acl']
        try:
            lastRevision = self.getRevList()[0]
        except IndexError:
            return security.AccessControlList(self.request.cfg)
        if self.rev == lastRevision:
            return self.pi['acl']

        return Page(self.request, self.page_name, rev=lastRevision).parseACL()

    # Text format -------------------------------------------------------

    def encodeTextMimeType(self, text):
        """ Encode text from moin internal representation to text/* mime type

        Make sure text uses CRLF line ends, keep trailing newline.

        @param text: text to encode (unicode)
        @rtype: unicode
        @return: encoded text
        """
        if text:
            lines = text.splitlines()
            # Keep trailing newline
            if text.endswith(u'\n') and not lines[-1] == u'':
                lines.append(u'')
            text = u'\r\n'.join(lines)
        return text

    def decodeTextMimeType(self, text):
        """ Decode text from text/* mime type to moin internal representation

        @param text: text to decode (unicode). Text must use CRLF!
        @rtype: unicode
        @return: text using internal representation
        """
        text = text.replace(u'\r', u'')
        return text

    def isConflict(self):
        """ Returns true if there is a known editing conflict for that page.

        @return: true if there is a known conflict.
        """

        cache = caching.CacheEntry(self.request, self, 'conflict', scope='item')
        return cache.exists()

    def setConflict(self, state):
        """ Sets the editing conflict flag.

        @param state: bool, true if there is a conflict.
        """
        cache = caching.CacheEntry(self.request, self, 'conflict', scope='item')
        if state:
            cache.update("") # touch it!
        else:
            cache.remove()


class RootPage(Page):
    """ These functions were removed from the Page class to remove hierarchical
        page storage support until after we have a storage api (and really need it).
        Currently, there is only 1 instance of this class: request.rootpage
    """
    def __init__(self, request):
        page_name = u''
        Page.__init__(self, request, page_name)

    def getPageBasePath(self, use_underlay=0):
        """ Get full path to a page-specific storage area. `args` can
            contain additional path components that are added to the base path.

        @param use_underlay: force using a specific pagedir, default 0:
                                1 = use underlay page dir
                                0 = use standard page dir
                                Note: we do NOT have special support for -1
                                      here, that will just behave as 0!
        @rtype: string
        @return: int underlay,
                 str the full path to the storage area
        """
        if self.cfg.data_underlay_dir is None:
            use_underlay = 0

        # 'auto' doesn't make sense here. maybe not even 'underlay':
        if use_underlay == 1:
            underlay, path = 1, self.cfg.data_underlay_dir
        # no need to check 'standard' case, we just use path in that case!
        else:
            # this is the location of the virtual root page
            underlay, path = 0, self.cfg.data_dir

        return underlay, path

    def getPageList(self, user=None, exists=1, filter=None, include_underlay=True, return_objects=False):
        """ List user readable pages under current page

        Currently only request.rootpage is used to list pages, but if we
        have true sub pages, any page can list its sub pages.

        The default behavior is listing all the pages readable by the
        current user. If you want to get a page list for another user,
        specify the user name.

        If you want to get the full page list, without user filtering,
        call with user="". Use this only if really needed, and do not
        display pages the user can not read.

        filter is usually compiled re match or search method, but can be
        any method that get a unicode argument and return bool. If you
        want to filter the page list, do it with this filter function,
        and NOT on the output of this function. page.exists() and
        user.may.read are very expensive, and should be done on the
        smallest data set.

        @param user: the user requesting the pages (MoinMoin.user.User)
        @param filter: filter function
        @param exists: filter existing pages
        @param include_underlay: determines if underlay pages are returned as well
        @param return_objects: lets it return a list of Page objects instead of
            names
        @rtype: list of unicode strings
        @return: user readable wiki page names
        """
        request = self.request
        request.clock.start('getPageList')
        # Check input
        if user is None:
            user = request.user

        # Get pages cache or create it
        cachedlist = request.cfg.cache.pagelists.getItem(request, 'all', None)
        if cachedlist is None:
            cachedlist = {}
            for name in self._listPages():
                # Unquote file system names
                pagename = wikiutil.unquoteWikiname(name)

                # Filter those annoying editor backups - current moin does not create
                # those pages any more, but users have them already in data/pages
                # until we remove them by a mig script...
                if pagename.endswith(u'/MoinEditorBackup'):
                    continue

                cachedlist[pagename] = None
            request.cfg.cache.pagelists.putItem(request, 'all', None, cachedlist)

        if user or exists or filter or not include_underlay or return_objects:
            # Filter names
            pages = []
            for name in cachedlist:
                # First, custom filter - exists and acl check are very
                # expensive!
                if filter and not filter(name):
                    continue

                page = Page(request, name)

                # Filter underlay pages
                if not include_underlay and page.getPageStatus()[0]: # is an underlay page
                    continue

                # Filter deleted pages
                if exists and not page.exists():
                    continue

                # Filter out page user may not read.
                if user and not user.may.read(name):
                    continue

                if return_objects:
                    pages.append(page)
                else:
                    pages.append(name)
        else:
            pages = cachedlist.keys()

        request.clock.stop('getPageList')
        return pages

    def getPageDict(self, user=None, exists=1, filter=None, include_underlay=True):
        """ Return a dictionary of filtered page objects readable by user

        Invoke getPageList then create a dict from the page list. See
        getPageList docstring for more details.

        @param user: the user requesting the pages
        @param filter: filter function
        @param exists: only existing pages
        @rtype: dict {unicode: Page}
        @return: user readable pages
        """
        pages = {}
        for name in self.getPageList(user=user, exists=exists, filter=filter, include_underlay=include_underlay):
            pages[name] = Page(self.request, name)
        return pages

    def _listPages(self):
        """ Return a list of file system page names

        This is the lowest level disk access, don't use it unless you
        really need it.

        NOTE: names are returned in file system encoding, not in unicode!

        @rtype: dict
        @return: dict of page names using file system encoding
        """
        # Get pages in standard dir
        path = self.getPagePath('pages')
        pages = self._listPageInPath(path)

        if self.cfg.data_underlay_dir is not None:
            # Merge with pages from underlay
            path = self.getPagePath('pages', use_underlay=1)
            underlay = self._listPageInPath(path)
            pages.update(underlay)

        return pages

    def _listPageInPath(self, path):
        """ List page names in domain, using path

        This is the lowest level disk access, don't use it unless you
        really need it.

        NOTE: names are returned in file system encoding, not in unicode!

        @param path: directory to list (string)
        @rtype: dict
        @return: dict of page names using file system encoding
        """
        pages = {}
        for name in os.listdir(path):
            # Filter non-pages in quoted wiki names
            # List all pages in pages directory - assume flat namespace.
            # We exclude everything starting with '.' to get rid of . and ..
            # directory entries. If we ever create pagedirs starting with '.'
            # it will be with the intention to have them not show up in page
            # list (like .name won't show up for ls command under UNIX).
            # Note that a . within a wiki page name will be quoted to (2e).
            if not name.startswith('.'):
                pages[name] = None

        if 'CVS' in pages:
            del pages['CVS'] # XXX DEPRECATED: remove this directory name just in
                             # case someone has the pages dir under CVS control.
        return pages

    def getPageCount(self, exists=0):
        """ Return page count

        The default value does the fastest listing, and return count of
        all pages, including deleted pages, ignoring acl rights.

        If you want to get a more accurate number, call with
        exists=1. This will be about 100 times slower though.

        @param exists: filter existing pages
        @rtype: int
        @return: number of pages
        """
        self.request.clock.start('getPageCount')
        if exists:
            # WARNING: SLOW
            pages = self.getPageList(user='')
        else:
            pages = self._listPages()
        count = len(pages)
        self.request.clock.stop('getPageCount')

        return count
