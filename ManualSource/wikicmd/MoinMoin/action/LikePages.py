# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - LikePages action

    This action generates a list of pages that either start or end
    with the same word as the current pagename. If only one matching
    page is found, that page is displayed directly.

    @copyright: 2001 Richard Jones <richard@bizarsoftware.com.au>,
                2001 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

import re

from MoinMoin import config, wikiutil
from MoinMoin.support import difflib
from MoinMoin.Page import Page


def execute(pagename, request):
    _ = request.getText
    start, end, matches = findMatches(pagename, request)

    # Error?
    if isinstance(matches, (str, unicode)):
        request.theme.add_msg(wikiutil.escape(matches), "info")
        Page(request, pagename).send_page()
        return

    # No matches
    if not matches:
        request.theme.add_msg(_('No pages like "%s"!') % (wikiutil.escape(pagename), ), "error")
        Page(request, pagename).send_page()
        return

    # One match - display it
    if len(matches) == 1:
        request.theme.add_msg(_('Exactly one page like "%s" found, redirecting to page.') % (wikiutil.escape(pagename), ), "info")
        Page(request, matches.keys()[0]).send_page()
        return

    # more than one match, list 'em
    # This action generate data using the user language
    request.setContentLanguage(request.lang)

    request.theme.send_title(_('Pages like "%s"') % (pagename), pagename=pagename)

    # Start content - IMPORTANT - without content div, there is no
    # direction support!
    request.write(request.formatter.startContent("content"))

    showMatches(pagename, request, start, end, matches)

    # End content and send footer
    request.write(request.formatter.endContent())
    request.theme.send_footer(pagename)
    request.theme.send_closing_html()

def findMatches(pagename, request, s_re=None, e_re=None):
    """ Find like pages

    @param pagename: name to match
    @param request: current reqeust
    @param s_re: start re for wiki matching
    @param e_re: end re for wiki matching
    @rtype: tuple
    @return: start word, end word, matches dict
    """
    # Get full list of pages, with no filtering - very fast. We will
    # first search for like pages, then filter the results.
    pages = request.rootpage.getPageList(user='', exists='')

    # Remove current page
    try:
        pages.remove(pagename)
    except ValueError:
        pass

    # Get matches using wiki way, start and end of word
    start, end, matches = wikiMatches(pagename, pages, start_re=s_re,
                                      end_re=e_re)

    # Get the best 10 close matches
    close_matches = {}
    found = 0
    for name in closeMatches(pagename, pages):
        # Skip names already in matches
        if name in matches:
            continue

        # Filter deleted pages or pages the user can't read
        page = Page(request, name)
        if page.exists() and request.user.may.read(name):
            close_matches[name] = 8
            found += 1
            # Stop after 10 matches
            if found == 10:
                break

    # Filter deleted pages or pages the user can't read from
    # matches. Order is important!
    for name in matches.keys(): # we need .keys() because we modify the dict
        page = Page(request, name)
        if not (page.exists() and request.user.may.read(name)):
            del matches[name]

    # Finally, merge both dicts
    matches.update(close_matches)

    return start, end, matches


def wikiMatches(pagename, pages, start_re=None, end_re=None):
    """
    Get pages that starts or ends with same word as this page

    Matches are ranked like this:
        4 - page is subpage of pagename
        3 - match both start and end
        2 - match end
        1 - match start

    @param pagename: page name to match
    @param pages: list of page names
    @param start_re: start word re (compile regex)
    @param end_re: end word re (compile regex)
    @rtype: tuple
    @return: start, end, matches dict
    """
    if start_re is None:
        start_re = re.compile('([%s][%s]+)' % (config.chars_upper,
                                               config.chars_lower))
    if end_re is None:
        end_re = re.compile('([%s][%s]+)$' % (config.chars_upper,
                                              config.chars_lower))

    # If we don't get results with wiki words matching, fall back to
    # simple first word and last word, using spaces.
    words = pagename.split()
    match = start_re.match(pagename)
    if match:
        start = match.group(1)
    else:
        start = words[0]

    match = end_re.search(pagename)
    if match:
        end = match.group(1)
    else:
        end = words[-1]

    matches = {}
    subpage = pagename + '/'

    # Find any matching pages and rank by type of match
    for name in pages:
        if name.startswith(subpage):
            matches[name] = 4
        else:
            if name.startswith(start):
                matches[name] = 1
            if name.endswith(end):
                matches[name] = matches.get(name, 0) + 2

    return start, end, matches


def closeMatches(pagename, pages):
    """ Get close matches.

    Return all matching pages with rank above cutoff value.

    @param pagename: page name to match
    @param pages: list of page names
    @rtype: list
    @return: list of matching pages, sorted by rank
    """
    # Match using case insensitive matching
    # Make mapping from lowerpages to pages - pages might have same name
    # with different case (although its stupid).
    lower = {}
    for name in pages:
        key = name.lower()
        if key in lower:
            lower[key].append(name)
        else:
            lower[key] = [name]

    # Get all close matches
    all_matches = difflib.get_close_matches(pagename.lower(), lower.keys(),
                                            len(lower), cutoff=0.6)

    # Replace lower names with original names
    matches = []
    for name in all_matches:
        matches.extend(lower[name])

    return matches


def showMatches(pagename, request, start, end, matches, show_count=True):
    keys = matches.keys()
    keys.sort()
    _showMatchGroup(request, matches, keys, 8, pagename, show_count)
    _showMatchGroup(request, matches, keys, 4, "%s/..." % pagename, show_count)
    _showMatchGroup(request, matches, keys, 3, "%s...%s" % (start, end), show_count)
    _showMatchGroup(request, matches, keys, 1, "%s..." % (start, ), show_count)
    _showMatchGroup(request, matches, keys, 2, "...%s" % (end, ), show_count)


def _showMatchGroup(request, matches, keys, match, title, show_count=True):
    _ = request.getText
    matchcount = matches.values().count(match)

    if matchcount:
        if show_count:
            # Render title line
            request.write(request.formatter.paragraph(1))
            request.write(request.formatter.strong(1))
            request.write(request.formatter.text(
                _('%(matchcount)d %(matches)s for "%(title)s"') % {
                    'matchcount': matchcount,
                    'matches': ' ' + (_('match'), _('matches'))[matchcount != 1],
                    'title': title}))
            request.write(request.formatter.strong(0))
            request.write(request.formatter.paragraph(0))

        # Render links
        request.write(request.formatter.bullet_list(1))
        for key in keys:
            if matches[key] == match:
                request.write(request.formatter.listitem(1))
                request.write(request.formatter.pagelink(1, key, generated=True))
                request.write(request.formatter.text(key))
                request.write(request.formatter.pagelink(0, key, generated=True))
                request.write(request.formatter.listitem(0))
        request.write(request.formatter.bullet_list(0))


