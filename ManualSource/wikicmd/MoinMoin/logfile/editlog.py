"""
    MoinMoin edit log class

    This is used for accessing the global edit-log (e.g. by RecentChanges) as
    well as for the local edit-log (e.g. PageEditor, info action).

    TODO:
    * when we have items with separate data and metadata storage, we do not
      need the local edit-log file any more (because everything in it will be
      stored into the revision's metadata).
    * maybe we can even get rid of the global edit-log as we know it now (and just
      maintaining a cache of recent changes metadata)

    @copyright: 2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin.logfile import LogFile
from MoinMoin import wikiutil, user, config
from MoinMoin.Page import Page

class EditLogLine:
    """
    Has the following attributes

    ed_time_usecs
    rev
    action
    pagename
    addr
    hostname
    userid
    extra
    comment
    """
    def __init__(self, usercache):
        self._usercache = usercache

    def __cmp__(self, other):
        try:
            return cmp(self.ed_time_usecs, other.ed_time_usecs)
        except AttributeError:
            return cmp(self.ed_time_usecs, other)

    def is_from_current_user(self, request):
        user = request.user
        if user.id:
            return user.id == self.userid
        return request.remote_addr == self.addr

    def getEditorData(self, request):
        """ Return a tuple of type id and string or Page object
            representing the user that did the edit.

            DEPRECATED - try to use getInterwikiEditorData
            NOT USED ANY MORE BY MOIN CODE!

            The type id is one of 'ip' (DNS or numeric IP), 'user' (user name)
            or 'homepage' (Page instance of user's homepage).
        """
        result = 'ip', request.cfg.show_hosts and self.hostname or ''
        if self.userid:
            if self.userid not in self._usercache:
                self._usercache[self.userid] = user.User(request, self.userid, auth_method="editlog:53")
            userdata = self._usercache[self.userid]
            if userdata.name:
                pg = wikiutil.getHomePage(request, username=userdata.name)
                if pg:
                    result = ('homepage', pg)
                else:
                    result = ('user', userdata.name)

        return result


    def getInterwikiEditorData(self, request):
        """ Return a tuple of type id and string or Page object
            representing the user that did the edit.

            The type id is one of 'ip' (DNS or numeric IP), 'user' (user name)
            or 'homepage' (Page instance of user's homepage) or 'anon' ('').
        """
        result = 'anon', ''
        if request.cfg.show_hosts and self.hostname:
            result = 'ip', self.hostname
        if self.userid:
            if self.userid not in self._usercache:
                self._usercache[self.userid] = user.User(request, self.userid, auth_method="editlog:75")
            userdata = self._usercache[self.userid]
            if userdata.mailto_author and userdata.email:
                return ('email', userdata.email)
            elif userdata.name:
                interwiki = wikiutil.getInterwikiHomePage(request, username=userdata.name)
                if interwiki:
                    result = ('interwiki', interwiki)
        return result


    def getEditor(self, request):
        """ Return a HTML-safe string representing the user that did the edit.
        """
        _ = request.getText
        if request.cfg.show_hosts and self.hostname and self.addr:
            title = " @ %s[%s]" % (self.hostname, self.addr)
        else:
            title = ""
        kind, info = self.getInterwikiEditorData(request)
        if kind == 'interwiki':
            name = self._usercache[self.userid].name
            aliasname = self._usercache[self.userid].aliasname
            if not aliasname:
                aliasname = name
            title = aliasname + title
            text = (request.formatter.interwikilink(1, title=title, generated=True, *info) +
                    request.formatter.text(name) +
                    request.formatter.interwikilink(0, title=title, *info))
        elif kind == 'email':
            name = self._usercache[self.userid].name
            aliasname = self._usercache[self.userid].aliasname
            if not aliasname:
                aliasname = name
            title = aliasname + title
            url = 'mailto:%s' % info
            text = (request.formatter.url(1, url, css='mailto', title=title) +
                    request.formatter.text(name) +
                    request.formatter.url(0))
        elif kind == 'ip':
            try:
                idx = info.index('.')
            except ValueError:
                idx = len(info)
            title = '???' + title
            text = request.formatter.text(info[:idx])
        elif kind == 'anon':
            title = ''
            text = _('anonymous')
        else:
            raise Exception("unknown EditorData type")
        return (request.formatter.span(1, title=title) +
                text +
                request.formatter.span(0))


class EditLog(LogFile):
    """ Used for accessing the global edit-log (e.g. by RecentChanges) as
        well as for the local edit-log (e.g. PageEditor, info action).
    """
    def __init__(self, request, filename=None, buffer_size=4096, **kw):
        if filename is None:
            rootpagename = kw.get('rootpagename', None)
            if rootpagename:
                filename = Page(request, rootpagename).getPagePath('edit-log', isfile=1)
            else:
                filename = request.rootpage.getPagePath('edit-log', isfile=1)
        LogFile.__init__(self, filename, buffer_size)
        self._NUM_FIELDS = 9
        self._usercache = {}

        # Used by antispam in order to show an internal name instead
        # of a confusing userid
        self.uid_override = kw.get('uid_override', None)

        # force inclusion of ip address, even if config forbids it.
        # this is needed so PageLock can work correctly for locks of anon editors.
        self.force_ip = kw.get('force_ip', False)

    def add(self, request, mtime, rev, action, pagename, host=None, extra=u'', comment=u''):
        """ Generate (and add) a line to the edit-log.

        If `host` is None, it's read from request vars.
        """
        if request.cfg.log_remote_addr or self.force_ip:
            if host is None:
                host = request.remote_addr or ''

            if request.cfg.log_reverse_dns_lookups:
                import socket
                try:
                    hostname = socket.gethostbyaddr(host)[0]
                    hostname = unicode(hostname, config.charset)
                except (socket.error, UnicodeError):
                    hostname = host
            else:
                hostname = host
        else:
            host = ''
            hostname = ''

        comment = wikiutil.clean_input(comment)
        user_id = request.user.valid and request.user.id or ''

        if self.uid_override is not None:
            user_id = ''
            hostname = self.uid_override
            host = ''

        line = u"\t".join((str(long(mtime)), # has to be long for py 2.2.x
                           "%08d" % rev,
                           action,
                           wikiutil.quoteWikinameFS(pagename),
                           host,
                           hostname,
                           user_id,
                           extra,
                           comment,
                           )) + "\n"
        self._add(line)

    def parser(self, line):
        """ Parse edit-log line into fields """
        fields = line.strip().split('\t')
        # Pad empty fields
        missing = self._NUM_FIELDS - len(fields)
        if missing:
            fields.extend([''] * missing)
        result = EditLogLine(self._usercache)
        (result.ed_time_usecs, result.rev, result.action,
         result.pagename, result.addr, result.hostname, result.userid,
         result.extra, result.comment, ) = fields[:self._NUM_FIELDS]
        if not result.hostname:
            result.hostname = result.addr
        result.pagename = wikiutil.unquoteWikiname(result.pagename.encode('ascii'))
        result.ed_time_usecs = long(result.ed_time_usecs or '0') # has to be long for py 2.2.x
        return result

    def set_filter(self, **kw):
        """ optionally filter for specific pagenames, addrs, hostnames, userids """
        expr = "1"
        for field in ['pagename', 'addr', 'hostname', 'userid']:
            if field in kw:
                expr = "%s and x.%s == %s" % (expr, field, repr(kw[field]))

        if 'ed_time_usecs' in kw:
            expr = "%s and long(x.ed_time_usecs) == %s" % (expr, long(kw['ed_time_usecs'])) # must be long for py 2.2.x

        self.filter = eval("lambda x: " + expr)


    def news(self, oldposition):
        """ What has changed in the edit-log since <oldposition>?
            Returns edit-log final position() and list of changed item names.
        """
        if oldposition is None:
            self.to_end()
        else:
            self.seek(oldposition)
        items = []
        for line in self:
            items.append(line.pagename)
            if line.action == 'SAVE/RENAME':
                items.append(line.extra) # == old page name

        newposition = self.position()
        logging.log(logging.NOTSET, "editlog.news: new pos: %r new items: %r", newposition, items)
        return newposition, items

