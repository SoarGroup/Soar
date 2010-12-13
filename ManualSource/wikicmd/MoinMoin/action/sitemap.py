# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - "sitemap" action

    Generate a URL list of all your pages (using google's sitemap XML format).

    @copyright: 2006-2008 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
import time
from MoinMoin import wikiutil

datetime_fmt = "%Y-%m-%dT%H:%M:%S+00:00"

def now():
    return time.strftime(datetime_fmt, time.gmtime())

def make_url_xml(request, vars):
    """ assemble a single <url> xml fragment """
    # add protocol:server - url must be complete path starting with/from /
    vars['url'] = request.getQualifiedURL(vars['url'])
    return """\
<url>
  <loc>%(url)s</loc>
  <lastmod>%(lastmod)s</lastmod>
  <changefreq>%(changefreq)s</changefreq>
  <priority>%(priority)s</priority>
</url>
""" % vars

def sitemap_url(request, page):
    """ return a sitemap <url>..</url> fragment for page object <page> """
    url = page.url(request)
    pagename = page.page_name
    lastmod = page.mtime_printable(request)
    if lastmod == "0": # can happen in case of errors
        lastmod = now()

    # page's changefreq, priority and lastmod depends on page type / name
    if pagename in [u"RecentChanges", u"TitleIndex", ]:
        # important dynamic pages with macros
        changefreq = "hourly"
        priority = "0.9"
        lastmod = now() # the page text mtime never changes, but the macro output DOES

    elif pagename in [request.cfg.page_front_page, ]:
        # important user edited pages
        changefreq = "hourly"
        priority = "1.0"

    elif wikiutil.isSystemPage(request, pagename):
        # other system pages are rather boring
        changefreq = "yearly"
        priority = "0.1"

    else:
        # these are the content pages:
        changefreq = "daily"
        priority = "0.5"

    return make_url_xml(request, locals())

def execute(pagename, request):
    _ = request.getText
    request.user.datetime_fmt = datetime_fmt

    request.mimetype = 'text/xml'

    # we emit a piece of data so other side doesn't get bored:
    request.write("""<?xml version="1.0" encoding="UTF-8"?>\r\n""")

    result = []
    result.append("""<urlset xmlns="http://www.google.com/schemas/sitemap/0.84">\n""")

    # we include the root url as an important and often changed URL
    rooturl = request.script_root + '/'
    result.append(make_url_xml(request, {
        'url': rooturl,
        'lastmod': now(), # fake
        'changefreq': 'hourly',
        'priority': '1.0',
    }))

    # Get page dict readable by current user
    try:
        underlay = int(request.values.get('underlay', 1))
    except ValueError:
        underlay = 1
    pages = request.rootpage.getPageDict(include_underlay=underlay)
    pagelist = pages.keys()
    pagelist.sort()
    for name in pagelist:
        result.append(sitemap_url(request, pages[name]))

    result.append("""</urlset>\n""")

    result = "".join(result)
    result = result.replace("\n", "\r\n") # text/* requires CR/LF

    # emit all real data
    request.write(result)

