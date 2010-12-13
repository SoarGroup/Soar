"""
    RSS Handling

    If you do changes, please check if it still validates after your changes:

    http://feedvalidator.org/

    @copyright: 2006-2007 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
import StringIO, re, time
from MoinMoin import wikixml, config, wikiutil
from MoinMoin.logfile import editlog
from MoinMoin.util import timefuncs
from MoinMoin.Page import Page
from MoinMoin.wikixml.util import RssGenerator

def full_url(request, page, querystr=None, anchor=None):
    url = page.url(request, anchor=anchor, querystr=querystr)
    url = wikiutil.escape(url)
    return request.getQualifiedURL(url)

def execute(pagename, request):
    """ Send recent changes as an RSS document
    """
    if not wikixml.ok:
        request.mimetype = 'text/plain'
        request.write("rss_rc action is not supported because of missing pyxml module.")
        return

    cfg = request.cfg

    # get params
    items_limit = 100
    try:
        max_items = int(request.values['items'])
        max_items = min(max_items, items_limit) # not more than `items_limit`
    except (KeyError, ValueError):
        # not more than 15 items in a RSS file by default
        max_items = 15
    try:
        unique = int(request.values.get('unique', 0))
    except ValueError:
        unique = 0
    try:
        diffs = int(request.values.get('diffs', 0))
    except ValueError:
        diffs = 0
    ## ddiffs inserted by Ralf Zosel <ralf@zosel.com>, 04.12.2003
    try:
        ddiffs = int(request.values.get('ddiffs', 0))
    except ValueError:
        ddiffs = 0

    # get data
    log = editlog.EditLog(request)
    logdata = []
    counter = 0
    pages = {}
    lastmod = 0
    for line in log.reverse():
        if not request.user.may.read(line.pagename):
            continue
        if (not line.action.startswith('SAVE') or
            ((line.pagename in pages) and unique)): continue
        #if log.dayChanged() and log.daycount > _MAX_DAYS: break
        line.editor = line.getInterwikiEditorData(request)
        line.time = timefuncs.tmtuple(wikiutil.version2timestamp(line.ed_time_usecs)) # UTC
        logdata.append(line)
        pages[line.pagename] = None

        if not lastmod:
            lastmod = wikiutil.version2timestamp(line.ed_time_usecs)

        counter += 1
        if counter >= max_items:
            break
    del log

    timestamp = timefuncs.formathttpdate(lastmod)
    etag = "%d-%d-%d-%d-%d" % (lastmod, max_items, diffs, ddiffs, unique)

    # for 304, we look at if-modified-since and if-none-match headers,
    # one of them must match and the other is either not there or must match.
    if request.if_modified_since == timestamp:
        if request.if_none_match:
            if request.if_none_match == etag:
                request.status_code = 304
        else:
            request.status_code = 304
    elif request.if_none_match == etag:
        if request.if_modified_since:
            if request.if_modified_since == timestamp:
                request.status_code = 304
        else:
            request.status_code = 304
    else:
        # generate an Expires header, using whatever setting the admin
        # defined for suggested cache lifetime of the RecentChanges RSS doc
        expires = time.time() + cfg.rss_cache

        request.mimetype = 'application/rss+xml'
        request.expires = expires
        request.last_modified = lastmod
        request.headers['Etag'] = etag

        # send the generated XML document
        baseurl = request.url_root

        logo = re.search(r'src="([^"]*)"', cfg.logo_string)
        if logo:
            logo = request.getQualifiedURL(logo.group(1))

        # prepare output
        out = StringIO.StringIO()
        handler = RssGenerator(out)

        # start SAX stream
        handler.startDocument()
        handler._out.write(
            '<!--\n'
            '    Add an "items=nnn" URL parameter to get more than the default 15 items.\n'
            '    You cannot get more than %d items though.\n'
            '    \n'
            '    Add "unique=1" to get a list of changes where page names are unique,\n'
            '    i.e. where only the latest change of each page is reflected.\n'
            '    \n'
            '    Add "diffs=1" to add change diffs to the description of each items.\n'
            '    \n'
            '    Add "ddiffs=1" to link directly to the diff (good for FeedReader).\n'
            '    Current settings: items=%i, unique=%i, diffs=%i, ddiffs=%i'
            '-->\n' % (items_limit, max_items, unique, diffs, ddiffs)
            )

        # emit channel description
        handler.startNode('channel', {
            (handler.xmlns['rdf'], 'about'): request.url_root,
            })
        handler.simpleNode('title', cfg.sitename)
        page = Page(request, pagename)
        handler.simpleNode('link', full_url(request, page))
        handler.simpleNode('description', 'RecentChanges at %s' % cfg.sitename)
        if logo:
            handler.simpleNode('image', None, {
                (handler.xmlns['rdf'], 'resource'): logo,
                })
        if cfg.interwikiname:
            handler.simpleNode(('wiki', 'interwiki'), cfg.interwikiname)

        handler.startNode('items')
        handler.startNode(('rdf', 'Seq'))
        for item in logdata:
            anchor = "%04d%02d%02d%02d%02d%02d" % item.time[:6]
            page = Page(request, item.pagename)
            link = full_url(request, page, anchor=anchor)
            handler.simpleNode(('rdf', 'li'), None, attr={(handler.xmlns['rdf'], 'resource'): link, })
        handler.endNode(('rdf', 'Seq'))
        handler.endNode('items')
        handler.endNode('channel')

        # emit logo data
        if logo:
            handler.startNode('image', attr={
                (handler.xmlns['rdf'], 'about'): logo,
                })
            handler.simpleNode('title', cfg.sitename)
            handler.simpleNode('link', baseurl)
            handler.simpleNode('url', logo)
            handler.endNode('image')

        # emit items
        for item in logdata:
            page = Page(request, item.pagename)
            anchor = "%04d%02d%02d%02d%02d%02d" % item.time[:6]
            rdflink = full_url(request, page, anchor=anchor)
            handler.startNode('item', attr={(handler.xmlns['rdf'], 'about'): rdflink, })

            # general attributes
            handler.simpleNode('title', item.pagename)
            if ddiffs:
                handler.simpleNode('link', full_url(request, page, querystr={'action': 'diff'}))
            else:
                handler.simpleNode('link', full_url(request, page))

            handler.simpleNode(('dc', 'date'), timefuncs.W3CDate(item.time))

            # description
            desc_text = item.comment
            if diffs:
                # TODO: rewrite / extend wikiutil.pagediff
                # searching for the matching pages doesn't really belong here
                revisions = page.getRevList()

                rl = len(revisions)
                for idx in range(rl):
                    rev = revisions[idx]
                    if rev <= item.rev:
                        if idx + 1 < rl:
                            lines = wikiutil.pagediff(request, item.pagename, revisions[idx+1], item.pagename, 0, ignorews=1)
                            if len(lines) > 20:
                                lines = lines[:20] + ['...\n']
                            lines = '\n'.join(lines)
                            lines = wikiutil.escape(lines)
                            desc_text = '%s\n<pre>\n%s\n</pre>\n' % (desc_text, lines)
                        break
            if desc_text:
                handler.simpleNode('description', desc_text)

            # contributor
            edattr = {}
            if cfg.show_hosts:
                edattr[(handler.xmlns['wiki'], 'host')] = item.hostname
            if item.editor[0] == 'interwiki':
                edname = "%s:%s" % item.editor[1]
                ##edattr[(None, 'link')] = baseurl + wikiutil.quoteWikiname(edname)
            else: # 'ip'
                edname = item.editor[1]
                ##edattr[(None, 'link')] = link + "?action=info"

            # this edattr stuff, esp. None as first tuple element breaks things (tracebacks)
            # if you know how to do this right, please send us a patch

            handler.startNode(('dc', 'contributor'))
            handler.startNode(('rdf', 'Description'), attr=edattr)
            handler.simpleNode(('rdf', 'value'), edname)
            handler.endNode(('rdf', 'Description'))
            handler.endNode(('dc', 'contributor'))

            # wiki extensions
            handler.simpleNode(('wiki', 'version'), "%i" % (item.ed_time_usecs))
            handler.simpleNode(('wiki', 'status'), ('deleted', 'updated')[page.exists()])
            handler.simpleNode(('wiki', 'diff'), full_url(request, page, querystr={'action': 'diff'}))
            handler.simpleNode(('wiki', 'history'), full_url(request, page, querystr={'action': 'info'}))
            # handler.simpleNode(('wiki', 'importance'), ) # ( major | minor )
            # handler.simpleNode(('wiki', 'version'), ) # ( #PCDATA )

            handler.endNode('item')

        # end SAX stream
        handler.endDocument()

        request.write(out.getvalue())

