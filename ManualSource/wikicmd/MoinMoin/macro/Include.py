# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Include macro

    This macro includes the formatted content of the given page(s). See

        http://purl.net/wiki/moinmaster/HelpOnMacros/Include

    for detailed docs.

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2000-2001 Richard Jones <richard@bizarsoftware.com.au>
    @license: GNU GPL, see COPYING for details.
"""

#Dependencies = ["pages"] # included page
Dependencies = ["time"] # works around MoinMoinBugs/TableOfContentsLacksLinks

generates_headings = True

import re, StringIO
from MoinMoin import wikiutil
from MoinMoin.Page import Page


_sysmsg = '<p><strong class="%s">%s</strong></p>'

## keep in sync with TableOfContents macro!
_arg_heading = r'(?P<heading>,)\s*(|(?P<hquote>[\'"])(?P<htext>.+?)(?P=hquote))'
_arg_level = r',\s*(?P<level>\d*)'
_arg_from = r'(,\s*from=(?P<fquote>[\'"])(?P<from>.+?)(?P=fquote))?'
_arg_to = r'(,\s*to=(?P<tquote>[\'"])(?P<to>.+?)(?P=tquote))?'
_arg_sort = r'(,\s*sort=(?P<sort>(ascending|descending)))?'
_arg_items = r'(,\s*items=(?P<items>\d+))?'
_arg_skipitems = r'(,\s*skipitems=(?P<skipitems>\d+))?'
_arg_titlesonly = r'(,\s*(?P<titlesonly>titlesonly))?'
_arg_editlink = r'(,\s*(?P<editlink>editlink))?'
_args_re_pattern = r'^(?P<name>[^,]+)(%s(%s)?%s%s%s%s%s%s%s)?$' % (
    _arg_heading, _arg_level, _arg_from, _arg_to, _arg_sort, _arg_items,
    _arg_skipitems, _arg_titlesonly, _arg_editlink)

_title_re = r"^(?P<heading>\s*(?P<hmarker>=+)\s.*\s(?P=hmarker))$"

def extract_titles(body, title_re):
    titles = []
    for title, _ in title_re.findall(body):
        h = title.strip()
        level = 1
        while h[level:level+1] == '=':
            level += 1
        title_text = h[level:-level].strip()
        titles.append((title_text, level))
    return titles

def execute(macro, text, args_re=re.compile(_args_re_pattern), title_re=re.compile(_title_re, re.M)):
    request = macro.request
    _ = request.getText

    # return immediately if getting links for the current page
    if request.mode_getpagelinks:
        return ''

    # parse and check arguments
    args = text and args_re.match(text)
    if not args:
        return (_sysmsg % ('error', _('Invalid include arguments "%s"!')) % (text, ))

    # prepare including page
    result = []
    print_mode = request.action in ("print", "format")
    this_page = macro.formatter.page
    if not hasattr(this_page, '_macroInclude_pagelist'):
        this_page._macroInclude_pagelist = {}

    # get list of pages to include
    inc_name = wikiutil.AbsPageName(this_page.page_name, args.group('name'))
    pagelist = [inc_name]
    if inc_name.startswith("^"):
        try:
            inc_match = re.compile(inc_name)
        except re.error:
            pass # treat as plain page name
        else:
            # Get user filtered readable page list
            pagelist = request.rootpage.getPageList(filter=inc_match.match)

    # sort and limit page list
    pagelist.sort()
    sort_dir = args.group('sort')
    if sort_dir == 'descending':
        pagelist.reverse()
    max_items = args.group('items')
    if max_items:
        pagelist = pagelist[:int(max_items)]

    skipitems = 0
    if args.group("skipitems"):
        skipitems = int(args.group("skipitems"))
    titlesonly = args.group('titlesonly')
    editlink = args.group('editlink')

    # iterate over pages
    for inc_name in pagelist:
        if not request.user.may.read(inc_name):
            continue
        if inc_name in this_page._macroInclude_pagelist:
            result.append(u'<p><strong class="error">Recursive include of "%s" forbidden</strong></p>' % (inc_name, ))
            continue
        if skipitems:
            skipitems -= 1
            continue
        fmt = macro.formatter.__class__(request, is_included=True)
        fmt._base_depth = macro.formatter._base_depth
        inc_page = Page(request, inc_name, formatter=fmt)
        if not inc_page.exists():
            continue
        inc_page._macroInclude_pagelist = this_page._macroInclude_pagelist

        # check for "from" and "to" arguments (allowing partial includes)
        body = inc_page.get_raw_body() + '\n'
        from_pos = 0
        to_pos = -1
        from_re = args.group('from')
        if from_re:
            try:
                from_match = re.compile(from_re, re.M).search(body)
            except re.error:
                ##result.append("*** fe=%s ***" % e)
                from_match = re.compile(re.escape(from_re), re.M).search(body)
            if from_match:
                from_pos = from_match.end()
            else:
                result.append(_sysmsg % ('warning', 'Include: ' + _('Nothing found for "%s"!')) % from_re)
        to_re = args.group('to')
        if to_re:
            try:
                to_match = re.compile(to_re, re.M).search(body, from_pos)
            except re.error:
                to_match = re.compile(re.escape(to_re), re.M).search(body, from_pos)
            if to_match:
                to_pos = to_match.start()
            else:
                result.append(_sysmsg % ('warning', 'Include: ' + _('Nothing found for "%s"!')) % to_re)

        if titlesonly:
            levelstack = []
            for title, level in extract_titles(body[from_pos:to_pos], title_re):
                if levelstack:
                    if level > levelstack[-1]:
                        result.append(macro.formatter.bullet_list(1))
                        levelstack.append(level)
                    else:
                        while levelstack and level < levelstack[-1]:
                            result.append(macro.formatter.bullet_list(0))
                            levelstack.pop()
                        if not levelstack or level != levelstack[-1]:
                            result.append(macro.formatter.bullet_list(1))
                            levelstack.append(level)
                else:
                    result.append(macro.formatter.bullet_list(1))
                    levelstack.append(level)
                result.append(macro.formatter.listitem(1))
                result.append(inc_page.link_to(request, title))
                result.append(macro.formatter.listitem(0))
            while levelstack:
                result.append(macro.formatter.bullet_list(0))
                levelstack.pop()
            continue

        if from_pos or to_pos != -1:
            inc_page.set_raw_body(body[from_pos:to_pos], modified=True)
        ##result.append("*** f=%s t=%s ***" % (from_re, to_re))
        ##result.append("*** f=%d t=%d ***" % (from_pos, to_pos))

        if not hasattr(request, "_Include_backto"):
            request._Include_backto = this_page.page_name

        # do headings
        level = None
        if args.group('heading') and args.group('hquote'):
            heading = args.group('htext') or inc_page.split_title()
            level = 1
            if args.group('level'):
                level = int(args.group('level'))
            if print_mode:
                result.append(macro.formatter.heading(1, level) +
                              macro.formatter.text(heading) +
                              macro.formatter.heading(0, level))
            else:
                url = inc_page.url(request)
                result.extend([
                    macro.formatter.heading(1, level, id=heading),
                    macro.formatter.url(1, url, css="include-heading-link"),
                    macro.formatter.text(heading),
                    macro.formatter.url(0),
                    macro.formatter.heading(0, level),
                ])

        # set or increment include marker
        this_page._macroInclude_pagelist[inc_name] = \
            this_page._macroInclude_pagelist.get(inc_name, 0) + 1

        # output the included page
        strfile = StringIO.StringIO()
        request.redirect(strfile)
        try:
            inc_page.send_page(content_only=True,
                               omit_footnotes=True,
                               count_hit=False)
            result.append(strfile.getvalue())
        finally:
            request.redirect()

        # decrement or remove include marker
        if this_page._macroInclude_pagelist[inc_name] > 1:
            this_page._macroInclude_pagelist[inc_name] = \
                this_page._macroInclude_pagelist[inc_name] - 1
        else:
            del this_page._macroInclude_pagelist[inc_name]

        # if no heading and not in print mode, then output a helper link
        if editlink and not (level or print_mode):
            result.extend([
                macro.formatter.div(1, css_class="include-link"),
                inc_page.link_to(request, '[%s]' % (inc_name, ), css_class="include-page-link"),
                inc_page.link_to(request, '[%s]' % (_('edit'), ), css_class="include-edit-link", querystr={'action': 'edit', 'backto': request._Include_backto}),
                macro.formatter.div(0),
            ])
        # XXX page.link_to is wrong now, it escapes the edit_icon html as it escapes normal text

    # return include text
    return ''.join(result)

# vim:ts=4:sw=4:et
